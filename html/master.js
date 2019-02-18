/* jshint browser: true */

function getDataBlock() {
  var server = encodeURI(document.getElementById('server').value);
  var book = encodeURI(document.getElementById('book').value);
  getDataBlockCore('html_master.php', 'book=' + book +'&server=' + server);
}

function getDataEvent() {
  var server = encodeURI(document.getElementById('server').value);
  var book = encodeURI(document.getElementById('book').value);
  getDataStreamCore("html_master.php?event=1&book=" + book +
                    "&server=" + server);
}
