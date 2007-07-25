/****************************************************************************

 Copyright (C) 2002-2006 Gilles Debunne (Gilles.Debunne@imag.fr)

This file is part of the QGLViewer library.
 Version 2.2.6-1, released on July 4, 2007.

 http://artis.imag.fr/Members/Gilles.Debunne/QGLViewer

 libQGLViewer is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 libQGLViewer is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with libQGLViewer; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#include "manipulatedFrameSetConstraint.h"
#include "frame.h"

using namespace qglviewer;

void ManipulatedFrameSetConstraint::clearSet()
{
   objects_.clear();
}

void ManipulatedFrameSetConstraint::addObjectToSet(PeakObject* o)
{
   objects_.append(o);
}

void ManipulatedFrameSetConstraint::constrainTranslation(qglviewer::Vec &translation, Frame *const)
{
   for (QList<PeakObject*>::iterator it=objects_.begin(), end=objects_.end(); it != end; ++it)
      (*it)->frame.translate(translation);
}

void ManipulatedFrameSetConstraint::constrainRotation(qglviewer::Quaternion &rotation, Frame *const frame)
{
   // A little bit of math. Easy to understand, hard to guess (tm).
   // rotation is expressed in the frame local coordinates system. Convert it back to world coordinates.
   const Vec worldAxis = frame->inverseTransformOf(rotation.axis());
   const Vec pos = frame->position();
   const float angle = rotation.angle();
   
   for (QList<PeakObject*>::iterator it=objects_.begin(), end=objects_.end(); it != end; ++it)
   {
      // Rotation has to be expressed in the object local coordinates system.
      Quaternion qObject((*it)->frame.transformOf(worldAxis), angle);
      (*it)->frame.rotate(qObject);
      
      // Comment these lines only rotate the objects
      Quaternion qWorld(worldAxis, angle);
      // Rotation around frame world position (pos)
      (*it)->frame.setPosition(pos + qWorld.rotate((*it)->frame.position() - pos));
   }
}
