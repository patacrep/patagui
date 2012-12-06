// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//******************************************************************************

import QtQuick 1.0

Rectangle {
    width: 300; height: 400

    Component {
        id: chordDelegate

        Item {
            width: 120; height: 120
	    Keys.onPressed: { 
		if (event.key == Qt.Key_Delete) {
		    console.log("delete key pressed")
		}
	    }

	    MouseArea {
		anchors.fill: parent
		onClicked: {
		    //console.log("diagram: " + name + " (" + id + ")" + " has been clicked");
		    view.currentIndex = index;
		}
            }

	    //chord name
	    Rectangle {
		id: chordNameRectangle
		y: 10;
	    	width: 80;	
		height: 25; 
		color: chordColor;
		radius: 4;
                anchors { horizontalCenter: parent.horizontalCenter }
	    }

            Text {
                anchors { verticalCenter: chordNameRectangle.verticalCenter ; horizontalCenter: parent.horizontalCenter }
		color: "white"; font.bold: true;
                text: name
            }

	    //diagram
	    Item{
		id: diagram
		width: 75; height: 60;
		y: 50
                anchors { horizontalCenter: parent.horizontalCenter }
		
		Component.onCompleted: {
		    var cellwidth = 15;
		    var cellheight = 15

		    //draw the diagram grid
		    var component = Qt.createComponent('Rectangle.qml');
		    for (var i=0; i < 4; i++) {
			var object = component.createObject(diagram);
			object.width  = 75;
			object.height = cellheight;
			object.x = 0;
			object.y = i * cellheight;
		    }
		    for (var i=0; i < strings.length-1; i++) {
			var object = component.createObject(diagram);
			object.width  = cellwidth;
			object.height = 60;
			object.x = i * cellwidth;
			object.y = 0;
		    }
		    //draw played strings
		    var circle = Qt.createComponent('Circle.qml');
		    var cross = Qt.createComponent('Cross.qml');
		    for (var i=0; i < strings.length; i++) {
			var stringValue = String(parseInt(strings[i]), 10);
			//console.log("reading string value " + stringValue + " from chord " + name);
			if (stringValue > 0) {
			    //played string
			    var object = circle.createObject(diagram);
			    object.x =  i*cellwidth - cellwidth/2.0 + 3;
			    object.y = (stringValue -1)*cellheight + 3;
			    object.color = "black";
			} else if (stringValue == 0) {
			    //empty string
			    var object = circle.createObject(diagram);
			    object.x = ( (i-1)*cellwidth)+cellwidth/2.0 +3;
			    object.y = -10;
			} else {
			    //non played string
			    var object = cross.createObject(diagram);
			    object.x = ( (i-1)*cellwidth)+cellwidth/2.0 +3;
			    object.y = -15;
			}
		    }
		}
	    }
        }
    }

    Component {
        id: chordHighlighter
        Rectangle { width: 120; height: 120; color: "lightsteelblue" }
    }

    GridView {
	id: view
        anchors.fill: parent
        cellWidth: 120; cellHeight: 120
        highlight: chordHighlighter
        focus: true
        model: chordModel
        delegate: chordDelegate
    }
}
