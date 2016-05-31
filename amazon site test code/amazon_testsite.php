<?php include "../inc/dbinfo.inc"; ?>
<html>
<body>
<h1>Sample page</h1>
<?php

  /* Connect to MySQL and select the database. */
  $connection = mysqli_connect(DB_SERVER, DB_USERNAME, DB_PASSWORD);

  if (mysqli_connect_errno()) echo "Failed to connect to MySQL: " . mysqli_connect_error();

  $database = mysqli_select_db($connection, DB_DATABASE);

  /* Ensure that the Employees table exists. */
  VerifyFlowTable($connection, DB_DATABASE); 
  /* If input fields are populated, add a row to the Flow table. */
  $flow_test = $_POST['Remove'];
  echo($_POST['Val']);
  if($flow_test=="Removed"){
	  echo("completed");
	  ClearFlow($connection);
  }else{
	   $date = date('Y-m-d H:i:s');
	   AddFlow($connection,$_POST['Val'],$date);
  }
?>
<!-- Input form -->
<form action="<?PHP echo $_SERVER['SCRIPT_NAME'] ?>" method="POST">
  <input type="submit" name="Remove" value="Removed" />
</form>
<!-- Display table data. -->
<table border="1" cellpadding="2" cellspacing="2">
  <tr>
    <td>ID</td>
    <td>Flow Total</td>
    <td>Time</td>
  </tr>
<?php

$result = mysqli_query($connection, "SELECT * FROM Data"); 

while($query_data = mysqli_fetch_row($result)) {
  echo "<tr>";
  echo "<td>",$query_data[0], "</td>",
       "<td>",$query_data[1], "</td>",
       "<td>",$query_data[2], "</td>";
  echo "</tr>";
}
?>
</table>

<!-- Clean up. -->
<?php

  mysqli_free_result($result);
  mysqli_close($connection);

?>

</body>
</html>

<?php

/* Add an employee to the table. */
function AddFlow($connection, $name, $address) {
   $n = mysqli_real_escape_string($connection, $name);
   $a = mysqli_real_escape_string($connection, $address);

   $query = "INSERT INTO `Data` (`Time`, `Flow`) VALUES ('$n', '$a');";

   if(!mysqli_query($connection, $query)) echo("<p>Error adding employee data.</p>");
}
/* Clear to the table. */
function ClearFlow($connection) {

   $query = "DELETE FROM `Data`";

   if(!mysqli_query($connection, $query)) echo("<p>Error adding employee data.</p>");
}
/* Check whether the table exists and, if not, create it. */
function VerifyFlowTable($connection, $dbName) {
  if(!TableExists("Data", $connection, $dbName)) 
  { 
     $query = "CREATE TABLE `Data` (
         `ID` int(11) NOT NULL AUTO_INCREMENT,
         `Time` varchar(45) DEFAULT NULL,
         `Flow` varchar(90) DEFAULT NULL,
         PRIMARY KEY (`ID`),
         UNIQUE KEY `ID_UNIQUE` (`ID`)
       ) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1";

     if(!mysqli_query($connection, $query)) echo("<p>Error creating table.</p>");
  }
}
/* Check for the existence of a table. */
function TableExists($tableName, $connection, $dbName) {
  $t = mysqli_real_escape_string($connection, $tableName);
  $d = mysqli_real_escape_string($connection, $dbName);

  $checktable = mysqli_query($connection, 
      "SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_NAME = '$t' AND TABLE_SCHEMA = '$d'");

  if(mysqli_num_rows($checktable) > 0) return true;

  return false;
}
?>
<script>
var xhr;
setInterval(function(){ getData(); }, 1000);
function processRequest(e) {
  if (xhr.readyState == 4 && xhr.status == 200) {
        var response = JSON.parse(xhr.responseText);
		var values = [response.result];
		post("SamplePageMk3.php",values,"post");
    }	
}
function getData() {
	xhr = new XMLHttpRequest();
	xhr.open('GET', "https://api.particle.io/v1/devices/2a003b001647343337363432/total?access_token=262a01b68bcb64095da7e08e31938ceb420727f1", true);
	xhr.send();
	xhr.onreadystatechange = processRequest;
}
function post(path, params, method) {
    method = method || "post"; // Set method to post by default if not specified.
	alert(params);
    // The rest of this code assumes you are not using a library.
    // It can be made less wordy if you use one.
    var form = document.createElement("form");
    form.setAttribute("method", method);
    form.setAttribute("action","<?PHP echo $_SERVER['SCRIPT_NAME'] ?>");

    for(var key in params) {
        if(params.hasOwnProperty(key)) {
            var hiddenField = document.createElement("input");
            hiddenField.setAttribute("type", "hidden");
            hiddenField.setAttribute("name", "Val");
            hiddenField.setAttribute("value", params[key]);

            form.appendChild(hiddenField);
         }
    }

    document.body.appendChild(form);
    form.submit();
}
</script>