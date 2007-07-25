######################################################################
# Automatically generated by qmake (2.01a) Fri Jul 6 16:45:56 2007
######################################################################

include ( ../Qt4Apps.pri )

# If you want to be able to load SDIF files produced by SPEAR, 
# download and install the open source Loris library from:
# http://www.cerlsoundgroup.org/Loris/
# 
# Then, uncomment the following line.
#
# CONFIG += LORIS_LIB

marsyasQGLVIEWER {

   TEMPLATE = app
   TARGET = PeakVis

   QT += xml opengl
   
	win32 {
		INCLUDEPATH += $$quote( "$$(QGLVIEWER)/QGLViewer" )	
		LIBS +=	-lQGLViewer
		CONFIG(release, debug|release) {
			LIBPATH += $$quote( "$$(QGLVIEWER)/QGLViewer/release" )
		}
		CONFIG(debug, debug|release) {
			LIBPATH += $$quote( "$$(QGLVIEWER)/QGLViewer/debug" )
		}
	}             

        unix {
        	LIBPATH+= /usr/local/lib
        	LIBS += -lqglviewer
        	DEFINES += MARSYAS_QGLVIEWER 
        	INCLUDEPATH += /usr/local/include/QGLViewer
        }


               
   # Input
   HEADERS += PeakVisMainWindow.h \
              manipulatedFrameSetConstraint.h \
              PeakFileCabinet.h \
              partialObject.h \
              peakObject.h \
              PeakVis.h 

   FORMS += PeakVisForm.ui 

   SOURCES += main.cpp \
              PeakVisMainWindow.cpp \
              manipulatedFrameSetConstraint.cpp \
              PeakFileCabinet.cpp \
              partialObject.cpp \
              peakObject.cpp \
              PeakVis.cpp 


   LORIS_LIB {
      DEFINES += LORIS_LIB
      unix:LIBS += -lloris
      unix:INCLUDEPATH += /usr/local/include/loris
   }

}
