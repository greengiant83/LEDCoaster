var fs = require("fs");
var vectorious = require('vectorious'),
    Vector = vectorious.Vector,
    Matrix = vectorious.Matrix;

var gravity = new Vector(0, 1); 
var coordinates = getAbsoluteCoordinates();
var path = analysePath(coordinates);
var precalc = getPrecalc(path, 150); //
savePrecalcToFile(precalc, "precalc.txt");

function getAcceleration(v)
{
	v = v.normalize();
	var acceleration = v.dot(gravity);
	return acceleration
}

function savePrecalcToFile(precalc, filename)
{
	var s = "";
	for(var i=0;i<precalc.length;i++)
	{
		s += precalc[i] + ",\n";
	}

	fs.writeFile(filename, s, function(err)
	{
		if(err) return console.log("Error saving file: ", err);
		console.log("Precalcs saved to: ", filename);
	})
}

function getPrecalc(path, ledCount)
{
	var c = 0;
	var precalc = [];

	for(var i=0;i<path.segments.length;i++)
	{
		var segment = path.segments[i];
		var ledLength = Math.round(segment.length / path.totalLength * ledCount);
		c += ledLength

		for(var j=0;j<ledLength;j++)
		{
			precalc.push(segment.acceleration);
		}
	}
	console.log("Final count: ", c);
	return precalc;
}

function analysePath(coords)
{
	var segments = [];
	var totalLength = 0;
	for(var i=1;i<coords.length;i++)
	{
		var delta = coords[i].subtract(coords[i-1]);
		var segment = {
			delta: delta,
			normal: delta.normalize(),
			length: delta.magnitude(),
			acceleration: getAcceleration(delta)
		}

		segments.push(segment);
		totalLength += segment.length;
	}
	var result = {
		segments: segments,
		totalLength: totalLength
	};

	return result;
}

function getAbsoluteCoordinates()
{
	//This is the massaged data from an svg's path.d attribute
	//Make sure your coordinates are absolute (File -> Inkscape Properties -> SVG Output -> Uncheck Allow relative coordinates) will come out with a capital commands like M and L
	return [
		new Vector(-294.96454,1634.2101),
new Vector(-270.72088,1517.0324),
new Vector(-299.00515,1319.0425),
new Vector(-343.45187,1003.8749),
new Vector(-379.81736,587.69203),
new Vector(-400.02041,155.34675),
new Vector(-404.06102,17.965999),
new Vector(-383.85797,-30.521323),
new Vector(-339.41125,-30.521323),
new Vector(-250.51783,62.412711),
new Vector(-92.934034,139.1843),
new Vector(60.609153,123.02186),
new Vector(214.15234,50.290881),
new Vector(270.72088,26.04722),
new Vector(315.16759,62.412711),
new Vector(412.14224,422.02702),
new Vector(501.03566,575.5702),
new Vector(634.3758,611.9357),
new Vector(759.63471,555.36715),
new Vector(840.44692,446.27068),
new Vector(892.97485,409.90519),
new Vector(973.78705,434.14885),
new Vector(1062.6805,478.59556),
new Vector(1187.9394,462.43312),
new Vector(1252.5892,349.29603),
new Vector(1272.7922,155.34675),
new Vector(1240.4673,-588.12553),
	];
}