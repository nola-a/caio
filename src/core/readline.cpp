/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of caio.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */
#include "readline.hpp"

#include <sys/types.h>
#include <sys/uio.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "logger.hpp"


namespace caio {
namespace readline {

History& History::operator=(History&& other)
{
    _histfname = other._histfname;
    _cursor = other._cursor;
    _current = other._current;
    _history = std::move(other._history);

    other._histfname = {};
    other._cursor = {};
    other._current = {};
    other._history = {};

    return *this;
}

std::string History::add_current()
{
    auto curr = _history[_current];
    if (!curr.empty()) {
        if (_current == 0 || curr != _history[_current - 1]) {
            if (_current == HISTSIZ) {
                std::copy(_history.begin() + 1, _history.end(), _history.begin());
            } else {
                ++_current;
            }
        }

        _cursor = _current;
        _history[_current] = {};
    }

    return curr;
}

void History::back()
{
    if (_cursor > 0) {
        --_cursor;
    }

    if (_current != _cursor) {
        _history[_current] = _history[_cursor];
    }
}

void History::forward()
{
    if (_cursor < _current) {
        ++_cursor;
    }

    if (_current != _cursor) {
        _history[_current] = _history[_cursor];
    }
}

std::string History::to_string() const
{
    std::ostringstream os{};

    for (size_t p = 0; p < _current; ++p) {
        os << _history[p] << (p == _cursor ? " <" : "") << std::endl;
    }

    return os.str();
}

void History::load()
{
    //TODO test
    if (!_histfname.empty()) {
        std::ifstream is{_histfname, std::ios::binary | std::ios::in};
        if (!is) {
            if (errno != ENOENT) {
                throw IOError{"Can't open: " + _histfname + ": " + Error::to_string()};
            }
            log.warn("Unable to load history file: %s: %s\n", _histfname.c_str(), Error::to_string(errno).c_str());
        }

        std::string line{};
        while (std::getline(is, line)) {
            if (line.empty()) {
                continue;
            }

            current() = line;
            add_current();
        }

        is.close();
    }
}

void History::save()
{
    //TODO test
    if (!_histfname.empty()) {
        std::ofstream os{_histfname, std::ios::binary | std::ios::out | std::ios::trunc};
        if (!os) {
            if (errno != ENOENT) {
                throw IOError{"Can't open: " + _histfname + ": " + Error::to_string()};
            }
            log.warn("Unable to save history file: %s: %s\n", _histfname.c_str(), Error::to_string(errno).c_str());
        }

        for (size_t pos = 0; pos < _current; ++pos) {
            os << _history[pos] << std::endl;
        }

        os.close();
    }
}

Readline::Readline(int ifd, int ofd, const std::string& histfname)
    : _ifd{::dup(ifd)},
      _ofd{::dup(ofd)},
      _history{histfname}
{
    if (_ifd < 0) {
        if (ifd >= 0) {
            throw IOError{"Can't duplicate input file descriptor: " + Error::to_string(errno)};
        } else {
            throw IOError{"Invalid input file descriptor"};
        }
    }

    if (_ofd < 0) {
        if (ofd >= 0) {
            throw IOError{"Can't duplicate output file descriptor: " + Error::to_string(errno)};
        } else {
            throw IOError{"Invalid output file descriptor"};
        }
    }

    term_init();
}

Readline& Readline::operator=(Readline&& other)
{
    close();
    _ifd = other._ifd;
    _ofd = other._ofd;
    _history = std::move(other._history);

    other._ifd = -1;
    other._ofd = -1;

    return *this;
}

void Readline::close()
{
    if (_ifd >= 0) {
        ::close(_ifd);
    }

    if (_ofd >= 0) {
        ::close(_ofd);
    }
}

void Readline::term_init()
{
    struct ::termios attr{};

    if (::tcgetattr(_ifd, &attr) < 0) {
        IOError{"Can't get input terminal attributes: " + Error::to_string(errno)};
    }

    attr.c_lflag &= ~(ICANON | ECHO);
    if (::tcsetattr(_ifd, TCSANOW, &attr) < 0) {
        IOError{"Can't set input terminal attributes: " + Error::to_string(errno)};
    }

    /* TODO: output */
}

char Readline::getc()
{
    char ch{};

    while (::read(_ifd, &ch, 1) <= 0) {
        if (errno != EINTR) {
            throw IOError{"Readline", "Can't read character: " + Error::to_string(errno)};
        }
    }

    return ch;
}

void Readline::write(char ch) const
{
    gsl::span<const char>buf{&ch, 1};
    write(buf);
}

void Readline::write(const std::string& msg) const
{
    gsl::span buf{msg.c_str(), msg.size()};
    write(buf);
}

void Readline::write(const gsl::span<const char>& data) const
{
    if (data.size() != 0) {
        auto wr = ::write(_ofd, data.data(), data.size());
        if (wr != data.size()) {
            throw IOError{"Readline", "Can't write: " + Error::to_string(errno)};
        }
    }
}

bool Readline::process_escape()
{
    char ch = getc();

    switch (ch) {
    case CURSOR_CONTROL:
        return process_cursor();

    default:;
    }

    return false;
}

bool Readline::process_cursor()
{
    char ch = getc();

    switch (ch) {
    case CURSOR_UP:
        _history.back();
        return true;

    case CURSOR_DOWN:
        _history.forward();
        return true;

    case CURSOR_RIGHT:
    case CURSOR_LEFT:
        return true;

    default:;
    }

    return false;
}

void Readline::erase_chars(size_t size)
{
    while (size--) {
        write(ERASE_PREV_CHAR);
    }
}

std::string Readline::getline()
{
    for (;;) {
        std::string& line = _history.current();
        char ch = getc();
        switch (ch) {
        case NEWLINE:
            write(ch);
            return _history.add_current();

        case BACKSPACE:
            if (!line.empty()) {
                write(ERASE_PREV_CHAR);
                line.pop_back();
            }
            break;

        case ESCAPE: {
            size_t size = line.size();
            if (process_escape()) {
                erase_chars(size);
                write(line);
            }
            break;
        }

        default:
            write(ch);
            line.push_back(ch);
        }
    }
}

}
}
