<?php
$raw = file_get_contents("php://stdin");
$arr = json_decode($raw, true);
/* sample data */
/*
$arr = array(
	'mpris:artUrl' => 'http://open.spotify.com/thumb/1714f099e924a6760d49837a261686ddaf7f291e',
	'mpris:length' => 241000000,
	'mpris:trackid' => 'spotify:track:203Sy2F2cr8kFWRAknaPdb',
	'xesam:album' => 'Follow The Reaper',
	'xesam:artist' => array(
		0 => 'Children Of Bodom',
	),
	'xesam:autoRating' => 0.37,
	'xesam:contentCreated' => '2008-01-01T00:00:00',
	'xesam:discNumber' => 1,
	'xesam:title' => 'Everytime I Die',
	'xesam:trackNumber' => 4,
	'xesam:url' => 'spotify:track:203Sy2F2cr8kFWRAknaPdb',
);
*/
$title = $arr["xesam:title"];
$artist = "";
$len = count($arr["xesam:artist"]);
$pos = 0;
foreach ($arr["xesam:artist"] as $a) {
	$pos += 1;
	$artist.= $a;
	if ($len - $pos == 1) {
		$artist.= "& ";
	} else if ($len - $pos != 0) {
		$artist.= ", ";
	}
}

echo $title . "\0" . $artist;
