// $Id: label_item.h,v 1.4 2004/02/03 00:13:26 arandell Exp $
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

#ifndef LABEL_ITEM_H
#define LABEL_ITEM_H

#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpalette.h>
#include <qmap.h>

#include "fixed.h"

typedef QMap<QString,QString> TextMap;

class QDomNodeList;
class QTextStream;

class LabelItem {
public:
    LabelItem();
    virtual ~LabelItem();

    // Type of item
    virtual QString type() const=0;

    // Painter environment
    QPen pen;
    QBrush brush;
    QFont font;
    double rotate;
    double scaleX;
    double scaleY;

    // Adjust for dpi
    virtual void dpiAdjust(double scaleX, double scaleY);

    // Draw methods which subclasses over-ride
    virtual void draw(QPainter* p);
    virtual void cleanup(QPainter* p);

    // Load and save in XML format
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);

    // Helpers
    void loadPen(QDomNodeList& nodes);
    void loadBrush(QDomNodeList& nodes);
    void loadFont(QDomNodeList& nodes);
};

class LineItem: public LabelItem
{
public:
    LineItem();
    ~LineItem();

    // Start and end points
    fixed x1, y1;
    fixed x2, y2;

    virtual QString type() const { return "Line"; }
    virtual void dpiAdjust(double scaleX, double scaleY);
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class RectItem: public LabelItem
{
public:
    RectItem();
    ~RectItem();

    // Position and size
    fixed x;
    fixed y;
    fixed width;
    fixed height;

    virtual QString type() const { return "Rect"; }
    virtual void dpiAdjust(double scaleX, double scaleY);
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class RoundRectItem: public RectItem
{
public:
    RoundRectItem();
    ~RoundRectItem();

    // Rounding factor (0 to 99 with 0 being straight angles)
    fixed xRound;
    fixed yRound;

    virtual QString type() const { return "RoundRect"; }
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class PanelItem: public RectItem
{
public:
    PanelItem();
    ~PanelItem();

    // Position and size
    QColorGroup colorGroup;
    bool sunken;
    int lineWidth;
    QBrush fill;

    virtual QString type() const { return "Panel"; }
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class EllipseItem: public RectItem
{
public:
    EllipseItem();
    ~EllipseItem();

    virtual QString type() const { return "Ellipse"; }
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class ArcItem: public RectItem
{
public:
    ArcItem();
    ~ArcItem();

    fixed angle;
    fixed arcLength;

    virtual QString type() const { return "Arc"; }
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class ChordItem: public RectItem
{
public:
    ChordItem();
    ~ChordItem();

    fixed angle;
    fixed arcLength;

    virtual QString type() const { return "Chord"; }
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class PolygonItem: public RectItem
{
public:
    PolygonItem();
    ~PolygonItem();

    // End points and fill method
    QPointArray points;
    bool winding;

    virtual QString type() const { return "Polygon"; }
    virtual void dpiAdjust(double scaleX, double scaleY);
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class PixmapItem: public LabelItem
{
public:
    PixmapItem();
    ~PixmapItem();

    // Position and pixmap to draw
    fixed x;
    fixed y;
    QPixmap pixmap;

    virtual QString type() const { return "Pixmap"; }
    virtual void dpiAdjust(double scaleX, double scaleY);
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class ImageItem: public LabelItem
{
public:
    ImageItem();
    ~ImageItem();

    // Position and pixmap to draw
    fixed x;
    fixed y;
    QImage image;

    virtual QString type() const { return "Image"; }
    virtual void dpiAdjust(double scaleX, double scaleY);
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);
};

class TextItem: public RectItem
{
public:
    TextItem();
    ~TextItem();

    QString text;
    int halign;
    int valign;
    bool wordWrap;

    virtual QString type() const { return "Text"; }
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);

    static TextMap mapping;
};

class BarcodeItem: public TextItem
{
public:
    BarcodeItem();
    ~BarcodeItem();

    // Formats supported
    enum { AUTO, UPC_A, UPC_E, EAN13, EAN8, CODE39, CODE93, CODE128,
	   I25, PLESSY, MSI_PLESSY, CODABAR };

    int format;
    double adjust;

    virtual QString type() const { return "Barcode"; }
    virtual void dpiAdjust(double scaleX, double scaleY);
    virtual void draw(QPainter* p);
    virtual void load(QDomNodeList& nodes);
    virtual void save(QTextStream& stream);

    // Methods to verify each type
    bool verifyUpcA();
    bool verifyUpcE();
    bool verifyEan13();
    bool verifyEan8();
    bool verifyCode39();
    bool verifyCode93();
    bool verifyCode128();
    bool verifyI25();
    bool verifyPlessy();
    bool verifyMSIPlessy();
    bool verifyCodabar();

    // Methods to encode each type to generic bar list
    bool encodeUpcA(QString& encoded, QStringList& numbers);
    bool encodeUpcE(QString& encoded, QStringList& numbers);
    bool encodeEan13(QString& encoded, QStringList& numbers);
    bool encodeEan8(QString& encoded, QStringList& numbers);
    bool encodeCode39(QString& encoded, QStringList& numbers);
    bool encodeCode93(QString& encoded, QStringList& numbers);
    bool encodeCode128(QString& encoded, QStringList& numbers);
    bool encodeI25(QString& encoded, QStringList& numbers);
    bool encodePlessy(QString& encoded, QStringList& numbers);
    bool encodeMSIPlessy(QString& encoded, QStringList& numbers);
    bool encodeCodabar(QString& encoded, QStringList& numbers);
};

#endif // LABEL_ITEM_H
