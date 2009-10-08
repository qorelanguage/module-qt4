/*
  qoresmokebinding.h

  Qore Programming Language Qt4 Module

  Copyright 2009 Qore Technologies sro

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef QORESMOKEBINDING_H
#define QORESMOKEBINDING_H


class QoreSmokeBinding : public SmokeBinding {

public:
    void deleted(Smoke::Index classId, void *obj);
    bool callMethod(Smoke::Index method, void *obj, Smoke::Stack args, bool isAbstract = false);
    char* className(Smoke::Index classId);

    static QoreSmokeBinding* Instance(Smoke * s) {
        if (!m_instance) {
            m_instance = new QoreSmokeBinding(s);
        }
        return m_instance;
    }

private:
    static QoreSmokeBinding * m_instance;

    QoreSmokeBinding(Smoke * s) : SmokeBinding(s) { };
    QoreSmokeBinding(const QoreSmokeBinding &);
    //QoreSmokeBinding& operator=(const QoreSmokeBinding&) {};

    ~QoreSmokeBinding() {
        delete m_instance;
    }
};


#endif
