<?php
/*

(c) Tunnler Project 2017

Going full yolo on this one.

Run using: `php -S localhost:8000 -t .`

*/

const rooms_file = "/tmp/rooms.json";
const timeout = 1 * 60; // seconds

// from https://gist.github.com/umidjons/42d378dc7d649c659a07
function randHash($len = 32) {
	return substr(md5(openssl_random_pseudo_bytes(20)), -$len);
}

function timestamp() {
  return time();
}

function filterRooms($rooms) {
  $deadline = timestamp() - timeout;
  foreach ($rooms as $token => $room) {
    if ($room['lastSeen'] <= $deadline) {  
      unset($rooms[$token]);
    }
  }

  return $rooms;
}

function loadRoomsFromDB() {
  if (!file_exists(rooms_file)) {
    return array();
  }
  $roomsJson = file_get_contents(rooms_file);
  $rooms = json_decode($roomsJson, TRUE);
  return filterRooms($rooms);
}

function saveRoomsToDB($rooms) {
  $roomsJson = json_encode($rooms, JSON_PRETTY_PRINT);
  file_put_contents(rooms_file, $roomsJson);
}

function getRooms() {
  $rooms = loadRoomsFromDB();
  saveRoomsToDB($rooms);

  // Create a new list which only exists data which the user is allowed to see
  $publicRoomsData = array();
  foreach ($rooms as $token => $room) {
    $publicRoomData = array(
      'server' => $room['server'],
      'serverPort' => $room['serverPort']
    );
    array_push($publicRoomsData, $publicRoomData);
  }

  return json_encode($publicRoomsData, JSON_PRETTY_PRINT);
}

function createRoom($server, $serverPort) {
  
  //FIXME: Do sanity checking on all arguments
  //FIXME: AT THE VERY LEAST CHECK FOR LARGE DATA HERE!

  $rooms = loadRoomsFromDB();
  
  // FIXME: Check if this server is already in the list / we don't want duplicates..
  if (false) {
    return '';
  }

  // Generate a valid token
  do {
    $token = randHash();
  } while(in_array($token, $rooms));

  // Add the room
  $room = array(
    'server' => $server,
    'serverPort' => $serverPort,
    'lastSeen' => timestamp()
  );
  $rooms = array_merge($rooms, array($token => $room));

  saveRoomsToDB($rooms);

  // Return the rooms token
  return $token;
}

function destroyRoom($token) {
  $rooms = loadRoomsFromDB();

  // Make sure the room even exists
  if (!in_array($token, $rooms)) {
    return 'not_found';
  }

  unset($rooms[$token]);

  saveRoomsToDB($rooms);
  return 'ok';
}

function updateRoom($token) {
  $rooms = loadRoomsFromDB();

  var_dump($rooms);

  // Make sure the room even exists
  if (!in_array($token, $rooms)) { //FIXME: Gets stuck in this
    return 'not_found';
  }

  $rooms[$token]['lastSeen'] = timestamp();

  saveRoomsToDB($rooms);
  return 'ok';
}
 
// get the HTTP method, path and body of the request
$method = $_SERVER['REQUEST_METHOD'];
//$request = explode('/', trim($_SERVER['PATH_INFO'], '/'));
$input = json_decode(file_get_contents('php://input'), true);
 
/*
// connect to the mysql database
$link = mysqli_connect('localhost', 'user', 'pass', 'dbname');
mysqli_set_charset($link,'utf8');
 
// retrieve the table and key from the path
$table = preg_replace('/[^a-z0-9_]+/i','',array_shift($request));
$key = array_shift($request)+0;
 
// escape the columns and values from the input object
$columns = preg_replace('/[^a-z0-9_]+/i','',array_keys($input));
$values = array_map(function ($value) use ($link) {
  if ($value===null) return null;
  return mysqli_real_escape_string($link,(string)$value);
},array_values($input));
 
// build the SET part of the SQL command
$set = '';
for ($i=0;$i<count($columns);$i++) {
  $set.=($i>0?',':'').'`'.$columns[$i].'`=';
  $set.=($values[$i]===null?'NULL':'"'.$values[$i].'"');
}
*/
 
// Do action based on HTTP method
$response = 'unknown';
switch ($method) {
  case 'GET':
    $response = getRooms();
    break;
  case 'PUT':
    //FIXME: Get 'server', 'serverPort'
    $response = createRoom($input['server'], $input['serverPort']);
    break;
  case 'POST':
    //FIXME: Get token
    $response = updateRoom($input['token']);
    break;
  case 'DELETE':
    //FIXME: Get token
    $response = destroyRoom($input['token']);
    break;
}

echo $response;

/* 
if (!$result) {
  http_response_code(404);
  die(mysqli_error());
}
*/
 
?>
