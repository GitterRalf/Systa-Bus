#include <QFile>
#include <QGraphicsPixmapItem>
#include <QMovie>


#include "aboutdialog.h"
#include "ui_aboutdialog.h"



AboutDialog::AboutDialog(QWidget *parent, QString version) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    connect ( ui->pb_OK, SIGNAL(clicked()), this, SLOT(hide()));

    ui->labelVersion->setText(version);
//    QGraphicsScene scene;
    QString resourcePath = ":/images/images/logo_g.gif";

    if( !QFile::exists(resourcePath) )
    {
      qDebug("*** Error - Resource path not found : \"%s\"",   resourcePath.toLatin1().data());
    }
    else
    {
      QMovie *movie = new QMovie(resourcePath);
      ui->label->setMovie(movie);
      movie->start();

//      QPixmap p1(resourcePath);

//      if ( p1.isNull() )
//      {
//        qDebug("*** Error - Null QPixmap");
//      }
//      else
//      {
//        QGraphicsPixmapItem *item_p1 = scene.addPixmap(p1);
//        item_p1->setVisible(true);
//        ui->graphicsView->setScene(&scene);
//        ui->graphicsView->show();
//      }
    }
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
