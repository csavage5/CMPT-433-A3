"use strict";

// get startup time
const start = new Date();
const startTime = start.getTime();

// Run when webpage fully loaded
$(document).ready(function() {
	// Register a callback function for the changeBtn button:	
	
});

// function changeBoxStyles() {
// 	console.log("Changing box styles.");
	
// 	// Change HTML making up the div:
// 	var name = $('#nameId').val();
// 	$('#box1').html("Hello <em>" + name + "</em>!");
	
// 	// Change HTML in div to be complex HTML:
// 	$('#box2').html(
// 			'<h3>An Idea!</h3>'+
// 			'<p><img src="bell.png" alt="" width="80px"/></p>' + 
// 			'<p>That\'s it!</p>');
	
// 	// Change the style of the element by writing css-like code:
// 	$('#box3').css({"border": "5px yellow dashed",
// 	               "color":   "red",
// 	               "backgroundColor": "green"});

// 	// Hide the element with id="box4"
// 	$('#box4').hide();
// }

function updateTime() {
	socket.emit('timeRequest');

	var timeStr = hours + ':' + minutes + ':' + seconds + "(H:M:S)";
	$('#timer').html(timeStr);
}