// Copyright (C) 2009-2013, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2013, Alexandre Dupas <alexandre.dupas@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//******************************************************************************

#ifndef __SINGLETON_HH__
#define __SINGLETON_HH__

/*!
  \file singleton.hh
  \class Singleton
  \brief Simple implementation of singleton design pattern

  Singleton class that ensures a single instance
  for the songs library.
*/

template <typename T>
class Singleton
{
protected:
    /// Constructor
    Singleton(){}

    /// Destructor
    ~Singleton(){}

public:
    /*!
    Returns the only instance of this class
  */
    static T *instance()
    {
        if (!_singleton)
            _singleton = new T;
        return static_cast<T*>(_singleton);
    }

private:
    static T *_singleton;
};

template <typename T>
T *Singleton<T>::_singleton = 0;

#endif // __SINGLETON_HH__
