#include "connector.h"
#include "modelitem.h"

#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QLineF>
#include <QPointF>
#include <QRectF>

namespace ResOptGui
{

Connector::Connector(ModelItem *startItem, ModelItem *endItem, bool active, QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsLineItem(parent, scene),
      p_start_item(startItem),
      p_end_item(endItem)
{



    if(active) setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else setPen(QPen(Qt::lightGray, 2, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));

    // setting the line
    updatePosition();

    // adding this connector to the items
    p_start_item->addOutletConnector(this);
    p_end_item->addInletConnector(this);
}


//-----------------------------------------------------------------------------------------------
// Updates the position of the line
//-----------------------------------------------------------------------------------------------
void Connector::updatePosition()
{

    // calculating the start position
    QPointF start_x = p_start_item->mapToScene(p_start_item->boundingRect().topRight());
    QPointF start_y = p_start_item->mapToScene(p_start_item->boundingRect().center());

    QPointF start(start_x.x(), start_y.y());


    // calculating the end position
    QPointF end_x = p_end_item->mapToScene(p_end_item->boundingRect().topLeft());
    QPointF end_y = p_end_item->mapToScene(p_end_item->boundingRect().center());

    QPointF end(end_x.x(), end_y.y());




    // Setting the line
    QLineF line(start, end);


    setLine(line);

}


} // namespace
