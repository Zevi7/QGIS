/***************************************************************************
    qgsuserinputwidget.h
     --------------------------------------
    Date                 : 04.2015
    Copyright            : (C) 2015 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsuserinputwidget.h"

#include <QFrame>

QgsUserInputWidget::QgsUserInputWidget( QWidget *parent )
  : QgsFloatingWidget( parent ? parent->window() : nullptr )
{
  //TODO add title tr( "User Input Panel" )

  QFrame *f = new QFrame();

  QPalette pal = palette();
  pal.setBrush( backgroundRole(), pal.window() );
  f->setPalette( pal );
  f->setAutoFillBackground( true );
  f->setFrameShape( QFrame::StyledPanel );
  f->setFrameShadow( QFrame::Plain );

  mLayout = new QBoxLayout( QBoxLayout::TopToBottom );
  mLayout->setAlignment( Qt::AlignRight | Qt::AlignTop );
  f->setLayout( mLayout );

  QBoxLayout *topLayout = new QBoxLayout( QBoxLayout::TopToBottom );
  topLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->addWidget( f );
  setLayout( topLayout );

  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
  hide();
}

void QgsUserInputWidget::addUserInputWidget( QWidget *widget )
{
  QFrame *line = nullptr;
  if ( mWidgetList.count() > 0 )
  {
    line = new QFrame( this );
    line->setFrameShadow( QFrame::Sunken );
    line->setFrameShape( mLayoutHorizontal ? QFrame::VLine : QFrame::HLine );
    mLayout->addWidget( line );
  }
  mLayout->addWidget( widget );

  connect( widget, &QObject::destroyed, this, &QgsUserInputWidget::widgetDestroyed );

  mWidgetList.insert( widget, line );

  show();
  raise();
  adjustSize();
}

void QgsUserInputWidget::widgetDestroyed( QObject *obj )
{
  if ( obj->isWidgetType() )
  {
    QWidget *w = qobject_cast<QWidget *>( obj );
    QMap<QWidget *, QFrame *>::iterator i = mWidgetList.find( w );
    while ( i != mWidgetList.end() )
    {
      if ( i.value() )
      {
        i.value()->deleteLater();
      }
      i = mWidgetList.erase( i );
    }
  }
  if ( mWidgetList.count() == 0 )
  {
    hide();
  }
}

void QgsUserInputWidget::setLayoutDirection( QBoxLayout::Direction direction )
{
  mLayout->setDirection( direction );

  bool horizontal = direction == QBoxLayout::LeftToRight || direction == QBoxLayout::RightToLeft;
  QMap<QWidget *, QFrame *>::const_iterator i = mWidgetList.constBegin();
  while ( i != mWidgetList.constEnd() )
  {
    if ( i.value() )
    {
      i.value()->setFrameShape( horizontal ? QFrame::VLine : QFrame::HLine );
    }
    ++i;
  }

  adjustSize();
}

void QgsUserInputWidget::paintEvent( QPaintEvent *event )
{
  if ( mWidgetList.count() == 0 )
  {
    hide();
  }
  else
  {
    QgsFloatingWidget::paintEvent( event );
  }
}
