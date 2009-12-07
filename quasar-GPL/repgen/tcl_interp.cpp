// $Id: tcl_interp.cpp,v 1.8 2005/02/15 08:19:36 bpepers Exp $
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

#include "tcl_interp.h"
#include <tcl.h>
#include <qdatetime.h>

TclInterp::TclInterp()
    : interp(NULL)
{
}

TclInterp::~TclInterp()
{
    if (interp != NULL)
	Tcl_DeleteInterp(interp);
}

void
TclInterp::initialize()
{
    if (interp != NULL)
	Tcl_DeleteInterp(interp);

    interp = Tcl_CreateInterp();
    runCallbacks("tcl");
}

static Tcl_Obj*
getObject(const QVariant& v)
{
    Tcl_Obj* value;
    QString text;

    switch (v.type()) {
    case QVariant::Int:
    case QVariant::UInt:
	value = Tcl_NewLongObj(v.toInt());
	break;
    case QVariant::Bool:
	value = Tcl_NewBooleanObj(v.toBool());
	break;
    case QVariant::Double:
	value = Tcl_NewDoubleObj(v.toDouble());
	break;
    case QVariant::Date:
	text = v.toDate().toString(Qt::ISODate);
	value = Tcl_NewStringObj(text.utf8(), text.utf8().length());
	break;
    default:
	text = v.toString();
	value = Tcl_NewStringObj(text.utf8(), text.utf8().length());
	break;
    }

    Tcl_IncrRefCount(value);
    return value;
}

static QVariant
getValue(Tcl_Obj* object)
{
    if (object == NULL) return QVariant();
    return QString::fromUtf8(Tcl_GetString(object));

// TODO: should use the type to return a more accurate QVariant
#if 0
    if (object->typePtr == NULL)
	return QString::fromUtf8(Tcl_GetString(object));

    QString type(object->typePtr->name);
    if (type == "int") {
	int val;
	if (Tcl_GetIntFromObj(NULL, object, &val) == TCL_OK)
	    return val;
    } else if (type == "double") {
	double val;
	if (Tcl_GetDoubleFromObj(NULL, object, &val) == TCL_OK)
	    return val;
    }

    return QString::fromUtf8(Tcl_GetString(object));
#endif
}

void
TclInterp::setVar(const QString& n, const QVariant& v)
{
    Tcl_Obj* name = getObject(n);
    Tcl_Obj* value = getObject(v);
    Tcl_ObjSetVar2(interp, name, NULL, value, 0);
}

void
TclInterp::setVar(const QString& t, const QString& n, const QVariant& v)
{
    Tcl_Obj* array = getObject(t);
    Tcl_Obj* element = getObject(n);
    Tcl_Obj* value = getObject(v);
    Tcl_ObjSetVar2(interp, array, element, value, 0);
}

QVariant
TclInterp::getVar(const QString& n)
{
    Tcl_Obj* name = getObject(n);
    Tcl_Obj* value = Tcl_ObjGetVar2(interp, name, NULL, 0);
    return getValue(value);
}

QVariant
TclInterp::getVar(const QString& t, const QString& n)
{
    Tcl_Obj* array = getObject(t);
    Tcl_Obj* element = getObject(n);
    Tcl_Obj* value = Tcl_ObjGetVar2(interp, array, element, 0);
    return getValue(value);
}

QVariant
TclInterp::evaluate(const QString& expr)
{
    if (expr.isEmpty()) return QVariant();

    Tcl_Obj* exprObj = getObject(expr);
    Tcl_Obj* resultObj = NULL;
    int result = Tcl_ExprObj(interp, exprObj, &resultObj);
    if (result != TCL_OK) {
	qWarning("Expr error: %s", Tcl_GetStringResult(interp));
	qWarning("      expr: " + expr);
	return QVariant();
    }

    QVariant value = getValue(resultObj);
    Tcl_DecrRefCount(resultObj);
    return value;
}

QVariant
TclInterp::execute(const QString& code)
{
    if (code.isEmpty()) return QVariant();

    Tcl_Obj* codeObj = getObject(code);
    int result = Tcl_EvalObjEx(interp, codeObj, TCL_EVAL_DIRECT);
    if (result != TCL_OK && result != TCL_RETURN) {
	QString trace = getVar("errorInfo").toString();
	qWarning("Script error: %s", Tcl_GetStringResult(interp));
	qWarning("Trace: " + trace);
	return QVariant();
    }

    return getValue(Tcl_GetObjResult(interp));
}
