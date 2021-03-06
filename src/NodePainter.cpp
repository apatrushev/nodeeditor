#include "NodePainter.hpp"

#include <QtCore/QMargins>

#include "NodeGeometry.hpp"
#include "NodeState.hpp"
#include "NodeDataModel.hpp"
#include "Node.hpp"

void
NodePainter::
paint(QPainter* painter,
      std::shared_ptr<Node> const &node)
{
  NodeGeometry const& geom = node->nodeGeometry();

  NodeState const& state = node->nodeState();

  geom.recalculateSize(painter->fontMetrics());

  //--------------------------------------------

  drawNodeRect(painter, geom);

  drawConnectionPoints(painter, geom, state);

  drawFilledConnectionPoints(painter, geom, state);

  auto const &model = node->nodeDataModel();

  drawModelName(painter, geom, state, model);

  drawEntryLabels(painter, geom, state, model);

  drawResizeRect(painter, geom, model);
}


void
NodePainter::
drawNodeRect(QPainter* painter,
             NodeGeometry const& geom)
{
  if (geom.hovered())
  {
    QPen p(Qt::white, 2.0);
    painter->setPen(p);
  }
  else
  {
    QPen p(Qt::white, 1.5);
    painter->setPen(p);
  }

  QLinearGradient gradient(QPointF(0.0, 0.0),
                           QPointF(2.0, geom.height()));

  QColor darkGray1 = QColor(Qt::gray).darker(200);
  QColor darkGray2 = QColor(Qt::gray).darker(250);

  gradient.setColorAt(0.0,  Qt::darkGray);
  gradient.setColorAt(0.03, darkGray1);
  gradient.setColorAt(0.97, darkGray2);
  gradient.setColorAt(1.0,  darkGray2.darker(110));

  painter->setBrush(gradient);

  unsigned int diam = geom.connectionPointDiameter();

  QRectF   boundary(0.0, 0.0, geom.width(), geom.height());
  QMargins m(diam, diam, diam, diam);

  double const radius = 3.0;

  painter->drawRoundedRect(boundary.marginsAdded(m), radius, radius);
}


void
NodePainter::
drawConnectionPoints(QPainter* painter,
                     NodeGeometry const& geom,
                     NodeState const& state)
{
  painter->setBrush(QColor(Qt::darkGray));

  auto diameter = geom.connectionPointDiameter();
  auto reducedDiameter = diameter * 0.6;

  auto drawPoints =
  [&](PortType portType)
  {
    size_t n = state.getEntries(portType).size();

    for (size_t i = 0; i < n; ++i)
    {

      QPointF p = geom.portScenePosition(i, portType);

      double r = 1.0;
      if (state.isReacting() &&
          !state.getEntries(portType)[i].lock())
      {
        auto   diff = geom.draggingPos() - p;
        double dist = std::sqrt(QPointF::dotProduct(diff, diff));

        double const thres = 40.0;

        r = (dist < thres) ?
            (2.0 - dist / thres ) :
            1.0;
      }

      painter->drawEllipse(p,
                           reducedDiameter * r,
                           reducedDiameter * r);
    }
  };

  drawPoints(PortType::Out);
  drawPoints(PortType::In);
}


void
NodePainter::
drawFilledConnectionPoints(QPainter* painter,
                           NodeGeometry const& geom,
                           NodeState const& state)
{
  painter->setPen(Qt::cyan);
  painter->setBrush(Qt::cyan);

  auto diameter = geom.connectionPointDiameter();

  auto drawPoints =
  [&](PortType portType)
  {
    size_t n = state.getEntries(portType).size();

    for (size_t i = 0; i < n; ++i)
    {
      QPointF p = geom.portScenePosition(i, portType);

      if (state.getEntries(portType)[i].lock())
      {
        painter->drawEllipse(p,
                             diameter * 0.4,
                             diameter * 0.4);
      }
    }
  };

  drawPoints(PortType::Out);
  drawPoints(PortType::In);
}


void
NodePainter::
drawModelName(QPainter* painter,
              NodeGeometry const& geom,
              NodeState const& state,
              std::unique_ptr<NodeDataModel> const & model)
{
  Q_UNUSED(state);

  QString const &name = model->modelName();

  if (name.isEmpty())
    return;

  QFont f = painter->font();

  f.setBold(true);

  QFontMetrics metrics(f);

  auto rect = metrics.boundingRect(name);

  QPointF position((geom.width() - rect.width()) / 2.0,
                   (geom.spacing() + geom.entryHeight()) / 3.0);

  painter->setFont(f);
  painter->setPen(Qt::white);
  painter->drawText(position, name);

  f.setBold(false);
  painter->setFont(f);
}


void
NodePainter::
drawEntryLabels(QPainter* painter,
                NodeGeometry const& geom,
                NodeState const& state,
                std::unique_ptr<NodeDataModel> const & model)
{
  QFontMetrics const & metrics =
    painter->fontMetrics();

  auto drawPoints =
  [&](PortType portType)
  {
    auto& entries = state.getEntries(portType);

    size_t n = entries.size();

    for (size_t i = 0; i < n; ++i)
    {

      QPointF p = geom.portScenePosition(i, portType);

      if (entries[i].expired())
        painter->setPen(Qt::darkGray);
      else
        painter->setPen(Qt::white);

      QString s = model->dataType(portType, i).name;

      auto rect = metrics.boundingRect(s);

      p.setY(p.y() + rect.height() / 4.0);

      switch (portType)
      {
        case PortType::In:
          p.setX(5.0);
          break;

        case PortType::Out:
          p.setX(geom.width() - 5.0 - rect.width());
          break;

        default:
          break;
      }

      painter->drawText(p, s);
    }
  };

  drawPoints(PortType::Out);

  drawPoints(PortType::In);
}


void
NodePainter::
drawResizeRect(QPainter* painter,
               NodeGeometry const& geom,
               std::unique_ptr<NodeDataModel> const & model)
{
  if (model->resizable())
  {
    painter->setBrush(Qt::gray);

    painter->drawEllipse(geom.resizeRect());
  }
}
