// $Id: messages.cpp,v 1.3 2004/02/02 22:42:03 arandell Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// Licensees holding a valid Quasar Commercial License may use this file
// in accordance with the Quasar Commercial License Agreement provided
// with the Software in the LICENSE.COMMERCIAL file.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.linuxcanada.com or email sales@linuxcanada.com for
// information about Quasar Accounting support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#include "messages.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qdom.h>
#include <assert.h>

Messages::Messages()
    : _codecName("ISO-8859-1")
{
}

Messages::~Messages()
{
}

static QString
numericEntity(int ch)
{
    return QString(ch <= 0x20 ? "<byte value=\"x%1\"/>" : "&#x%1;")
	.arg(ch, 0, 16);
}

static QString
protect(const QCString& str)
{
    QString result;
    int len = (int) str.length();
    for (int k = 0; k < len; k++) {
	switch (str[k]) {
	case '\"':
	    result += QString("&quot;");
	    break;
	case '&':
	    result += QString("&amp;");
	    break;
	case '>':
	    result += QString("&gt;");
	    break;
	case '<':
	    result += QString("&lt;");
	    break;
	case '\'':
	    result += QString("&apos;");
	    break;
	default:
	    if (uchar(str[k]) < 0x20 && str[k] != '\n')
		result += numericEntity(uchar(str[k]));
	    else
		result += str[k];
	}
    }
    return result;
}

static QString
evilBytes(const QCString& str, bool utf8=false)
{
    if (utf8) {
	return protect(str);
    } else {
	QString result;
	QCString t = protect(str).latin1();
	int len = int(t.length());
	for (int k = 0; k < len; k++) {
	    if (uchar(t[k]) >= 0x7f)
		result += numericEntity(uchar(t[k]));
	    else
		result += QChar(t[k]);
	}
	return result;
    }
}

void
Message::load(QDomNodeList& nodes)
{
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "source") {
	    source = text;
	} else if (tag == "translation") {
	    translation = text;
	    type = e.attribute("type");
	} else {
	    qWarning("Error: unknown message tag: " + tag);
	}
    }
}

void
Context::load(QDomNodeList& nodes)
{
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "name") {
	    name = text;
	} else if (tag == "message") {
	    QDomNodeList nodes = e.childNodes();
	    Message message;
	    message.load(nodes);
	    messages.push_back(message);
	} else {
	    qWarning("Error: unknown context tag: " + tag);
	}
    }
}

void
Context::merge(QDomNodeList& nodes)
{
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "name") {
	    assert(text == name);
	} else if (tag == "message") {
	    QDomNodeList nodes = e.childNodes();
	    Message message;
	    message.load(nodes);
	    if (messageExists(message.source)) {
		Message& current = findMessage(message.source);
		current.translation = message.translation;
		current.type = message.type;
	    }
	} else {
	    qWarning("Error: unknown context tag: " + tag);
	}
    }
}

bool
Context::messageExists(const QString& source)
{
    for (unsigned int i = 0; i < messages.size(); ++i)
	if (messages[i].source == source)
	    return true;
    return false;
}

Message&
Context::findMessage(const QString& source)
{
    for (unsigned int i = 0; i < messages.size(); ++i)
	if (messages[i].source == source)
	    return messages[i];

    Message message;
    message.source = source;
    return findMessage(source);
}

bool
Messages::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) return false;

    QDomDocument doc;
    if (!doc.setContent(&file)) return false;
    file.close();

    QDomElement root = doc.documentElement();
    for (uint i = 0; i < root.childNodes().count(); ++i) {
	QDomNode node = root.childNodes().item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "context") {
	    QDomNodeList nodes = e.childNodes();
	    Context context;
	    context.load(nodes);
	    _contexts.push_back(context);
	} else {
	    qWarning("Error: unknown root tag: " + tag);
	}
    }

    return true;
}

bool
Messages::merge(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) return false;

    QDomDocument doc;
    if (!doc.setContent(&file)) return false;
    file.close();

    QDomElement root = doc.documentElement();
    for (uint i = 0; i < root.childNodes().count(); ++i) {
	QDomNode node = root.childNodes().item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "context") {
	    QDomNodeList nodes = e.childNodes();
	    QDomElement n = nodes.item(0).toElement();
	    assert(n.tagName() == "name");
	    QString name = n.text();

	    if (contextExists(name)) {
		Context& context = findContext(name);
		context.merge(nodes);
	    }
	} else {
	    qWarning("Error: unknown root tag: " + tag);
	}
    }

    return true;
}

bool
Messages::save(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(IO_WriteOnly)) return false;

    QTextStream stream(&file);
    //stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));

    stream << "<!DOCTYPE TS><TS>\n";
    if (_codecName != "ISO-8859-1")
	stream << "<defaultcodec>" << _codecName << "</defaultcodec>\n";

    for (unsigned int i = 0; i < _contexts.size(); ++i) {
	const Context& context = _contexts[i];
	QCString name = context.name.local8Bit();

	stream << "<context>" << endl;
	stream << "  <name>" << evilBytes(name) << "</name>" << endl;

	for (unsigned int j = 0; j < context.messages.size(); ++j) {
	    const Message& message = context.messages[j];
	    stream << "  <message>" << endl;
	    stream << "    <source>" << evilBytes(message.source.local8Bit())
		   << "</source>" << endl;
	    stream << "    <translation type=\"" << message.type
		   << "\">" << evilBytes(message.translation.local8Bit())
		   << "</translation>" << endl;
	    stream << "  </message>" << endl;
	}
	stream << "</context>" << endl;
    }
    stream << "</TS>" << endl;

    return true;
}

bool
Messages::contextExists(const QString& name) const
{
    for (unsigned int i = 0; i < _contexts.size(); ++i)
	if (_contexts[i].name == name)
	    return true;
    return false;
}

Context&
Messages::findContext(const QString& name)
{
    for (unsigned int i = 0; i < _contexts.size(); ++i)
	if (_contexts[i].name == name)
	    return _contexts[i];

    Context context;
    context.name = name;
    _contexts.push_back(context);

    return findContext(name);
}
