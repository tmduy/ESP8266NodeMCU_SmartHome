<?php
//Creates new record as per request
    //Connect to database
    $servername = "localhost";
    $username = "root";
    $password = "";
    $dbname = "nodemcu_ldr";

    // Create connection
    $conn = new mysqli($servername, $username, $password, $dbname);
    // Check connection
    if ($conn->connect_error) {
        die("Database Connection failed: " . $conn->connect_error);
    }

    //Get current date and time
    date_default_timezone_set('Asia/Jakarta');
    $d = date("Y-m-d");
    $t = date("H:i:s");

    if(!empty($_POST['status']))
    {
		$status = $_POST['status'];

	    $sql = "INSERT INTO nodemcu_ldr_table (Status, Date, Time)
		
		VALUES ('".$status."', '".$d."', '".$t."')";

		if ($conn->query($sql) === TRUE) {
		    echo "OK";
		} else {
		    echo "Error: " . $sql . "<br>" . $conn->error;
		}
	}


	$conn->close();
?>