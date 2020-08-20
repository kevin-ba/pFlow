# OutlineFlow

OutlineFlow is a program with which you can load a room plan as an image and then draw the room off to get the coordinates.

## Developer tools

Programmed with C++ and Qt

Development environment:
Qt Creator 4.12.4

Qt-Version:
Qt 5.15.0

(When installing on Windows you also have to tick MinGW during the installation)

## Functionality
* The room drawing is done by creating a polygon and adding the points to the polygon with the help of the classes *QPolygon* and *QPoint*.

* The points are obtained by mouse events with the help of the class *QMouseEvent*.

* When a mouse click exceeds a certain distance from its closest point, a new point is created and added to the QPolygon of the room. If the mouse click is placed very close to a point or on a point, the point can be moved or removed in case of a double click.       
This functionality is implemented with the *getClosestPoint()* method and the calculation of the manhattan distance between the closest point and the mouse click.

* When inserting a point, the distance from the point to each segment is calculated (*distToSegment()*). This is used to find out where the point has to be inserted.
