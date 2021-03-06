/***************************************************************************
                            qgsscalebarrenderer.cpp
                            -----------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco.hugentobler@karto.baug.ethz.ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscalebarrenderer.h"
#include "qgsscalebarsettings.h"
#include "qgslayoututils.h"
#include <QFontMetricsF>
#include <QPainter>

void QgsScaleBarRenderer::drawDefaultLabels( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
  {
    return;
  }

  QPainter *painter = context.painter();

  painter->save();

  QFont scaledFont = settings.font();
  scaledFont.setPointSizeF( scaledFont.pointSizeF() * context.scaleFactor() );
  painter->setPen( QPen( settings.fontColor() ) );

  QString firstLabel = firstLabelString( settings );
  double xOffset = context.convertToPainterUnits( QgsLayoutUtils::textWidthMM( settings.font(), firstLabel ) / 2, QgsUnitTypes::RenderMillimeters );

  double currentLabelNumber = 0.0;

  int nSegmentsLeft = settings.numberOfSegmentsLeft();
  int segmentCounter = 0;
  QString currentNumericLabel;

  QList<double> positions = segmentPositions( scaleContext, settings );

  for ( int i = 0; i < positions.size(); ++i )
  {
    if ( segmentCounter == 0 && nSegmentsLeft > 0 )
    {
      //label first left segment
      currentNumericLabel = firstLabel;
    }
    else if ( segmentCounter != 0 && segmentCounter == nSegmentsLeft ) //reset label number to 0 if there are left segments
    {
      currentLabelNumber = 0;
    }

    if ( segmentCounter >= nSegmentsLeft )
    {
      currentNumericLabel = QString::number( currentLabelNumber / settings.mapUnitsPerScaleBarUnit() );
    }

    if ( segmentCounter == 0 || segmentCounter >= nSegmentsLeft ) //don't draw label for intermediate left segments
    {
      QgsLayoutUtils::drawText( painter, QPointF( context.convertToPainterUnits( positions.at( i ) - QgsLayoutUtils::textWidthMM( settings.font(), currentNumericLabel ) / 2, QgsUnitTypes::RenderMillimeters ) + xOffset,
                                context.convertToPainterUnits( QgsLayoutUtils::fontAscentMM( settings.font() ) + settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters ) ),
                                currentNumericLabel, scaledFont, settings.fontColor() );
    }

    if ( segmentCounter >= nSegmentsLeft )
    {
      currentLabelNumber += settings.unitsPerSegment();
    }
    ++segmentCounter;
  }

  //also draw the last label
  if ( !positions.isEmpty() )
  {
    currentNumericLabel = QString::number( currentLabelNumber / settings.mapUnitsPerScaleBarUnit() );
    QgsLayoutUtils::drawText( painter, QPointF( context.convertToPainterUnits( positions.at( positions.size() - 1 ) + scaleContext.segmentWidth - QgsLayoutUtils::textWidthMM( settings.font(), currentNumericLabel ) / 2, QgsUnitTypes::RenderMillimeters ) + xOffset,
                              context.convertToPainterUnits( QgsLayoutUtils::fontAscentMM( settings.font() ) + settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters ) ),
                              currentNumericLabel + ' ' + settings.unitLabel(), scaledFont, settings.fontColor() );
  }

  painter->restore();
}

QSizeF QgsScaleBarRenderer::calculateBoxSize( const QgsScaleBarSettings &settings,
    const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  //consider centered first label
  double firstLabelLeft = QgsLayoutUtils::textWidthMM( settings.font(), firstLabelString( settings ) ) / 2;

  //consider last number and label

  double largestLabelNumber = settings.numberOfSegments() * settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit();
  QString largestNumberLabel = QString::number( largestLabelNumber );
  QString largestLabel = QString::number( largestLabelNumber ) + ' ' + settings.unitLabel();
  double largestLabelWidth = QgsLayoutUtils::textWidthMM( settings.font(), largestLabel ) - QgsLayoutUtils::textWidthMM( settings.font(), largestNumberLabel ) / 2;

  double totalBarLength = scaleContext.segmentWidth * ( settings.numberOfSegments() + ( settings.numberOfSegmentsLeft() > 0 ? 1 : 0 ) );

  double width = firstLabelLeft + totalBarLength + 2 * settings.pen().widthF() + largestLabelWidth + 2 * settings.boxContentSpace();
  double height = settings.height() + settings.labelBarSpace() + 2 * settings.boxContentSpace() + QgsLayoutUtils::fontAscentMM( settings.font() );

  return QSizeF( width, height );
}

QString QgsScaleBarRenderer::firstLabelString( const QgsScaleBarSettings &settings ) const
{
  if ( settings.numberOfSegmentsLeft() > 0 )
  {
    return QString::number( settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit() );
  }
  else
  {
    return QStringLiteral( "0" );
  }
}

double QgsScaleBarRenderer::firstLabelXOffset( const QgsScaleBarSettings &settings ) const
{
  QString firstLabel = firstLabelString( settings );
  return QgsLayoutUtils::textWidthMM( settings.font(), firstLabel ) / 2.0;
}

QList<double> QgsScaleBarRenderer::segmentPositions( const ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const
{
  QList<double> positions;

  double currentXCoord = settings.pen().widthF() + settings.boxContentSpace();

  //left segments
  double leftSegmentSize = scaleContext.segmentWidth / settings.numberOfSegmentsLeft();
  for ( int i = 0; i < settings.numberOfSegmentsLeft(); ++i )
  {
    positions << currentXCoord;
    currentXCoord += leftSegmentSize;
  }

  //right segments
  for ( int i = 0; i < settings.numberOfSegments(); ++i )
  {
    positions << currentXCoord;
    currentXCoord += scaleContext.segmentWidth;
  }
  return positions;
}

QList<double> QgsScaleBarRenderer::segmentWidths( const ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const
{
  QList<double> widths;

  //left segments
  if ( settings.numberOfSegmentsLeft() > 0 )
  {
    double leftSegmentSize = scaleContext.segmentWidth / settings.numberOfSegmentsLeft();
    for ( int i = 0; i < settings.numberOfSegmentsLeft(); ++i )
    {
      widths << leftSegmentSize;
    }
  }

  //right segments
  for ( int i = 0; i < settings.numberOfSegments(); ++i )
  {
    widths << scaleContext.segmentWidth;
  }

  return widths;
}
