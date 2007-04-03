#include <QApplication>
#include <QPushButton>
#include <math.h>
#include <iostream>
#include <string>

#include "Marx2DGraph.h"
#include "MarMonitors.h"

int main(int argc, char *argv[])
{

  QApplication app(argc, argv);


  std::string audioFile = "/home/gtzan/data/sound/music_speech/music/gravity.au";

  MarMonitors* monitors = new MarMonitors();
  
  monitors->setGeometry(50, 50, 800, 500);
  monitors->show();

  return app.exec();
}
