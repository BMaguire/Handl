
window.onload = function() {

  var deviceID = "2c002d001447363333343437";
  var accessToken = "e1225be23d29d7b7809c1f0cc657de251f2399b8";
  var eventSource = new EventSource("https://api.particle.io/v1/devices/" + deviceID + "/events/?access_token=" + accessToken);

  //document.getElementById("temp").innerHTML = "Waiting for data...";

  eventSource.addEventListener('open', function(e) {
            console.log("Opened!"); },false);

  eventSource.addEventListener('error', function(e) {
      console.log("Errored!"); },false);
  //
  eventSource.addEventListener('log', function(e) {
    var parsedData = JSON.parse(e.data);
    addRow(parsedData.data, parsedData.published_at);
  }, false);

}

 // ADD A NEW ROW TO THE TABLE.s
 function addRow(name, time) {
   var table = document.getElementById("myTable");
   var rowCnt = table.rows.length;        // GET TABLE ROW COUNT
   var row = table.insertRow(rowCnt);
   var cell1 = row.insertCell(0);
   var cell2 = row.insertCell(1);
   cell1.innerHTML = name;
   cell2.innerHTML = time;
 }
