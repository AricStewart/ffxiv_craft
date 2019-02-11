/* jshint browser: true */

function showSpinner() {
  var dlg = document.getElementById('refresh_spinner');
  if (!dlg) {
    dlg = document.createElement('DIV');
    dlg.classList.add('modal');
    dlg.id = 'refresh_spinner';
    var content = document.createElement('DIV');
    dlg.appendChild(content);
    content.classList.add('modal-content');
    content.classList.add('jumbo');
    content.classList.add('center');
    content.classList.add('round-xxlarge');
    content.style.width = '90px';
    content.style.backgroundColor = 'orange';
    var span = document.createElement('SPAN');
    span.innerHTML = '<i class="fa fa-refresh spin"></i>';
    content.appendChild(span);
    document.body.appendChild(dlg);
  }
  dlg.style.display = 'block';
}

function hideSpinner() {
  document.getElementById('refresh_spinner').style.display = 'none';
}

function fillBookFrame(dataset)
{
    dataset.forEach(function(data) {
        if (data.Cost.Market <= 0) {
            marketCost =  'UNAVAILABLE';
        } else {
            marketCost = data.Cost.Market.toLocaleString() + " gil";
        }

        if (data.Cost.Optimal <= 0) {
            optimalCost =  'UNAVAILABLE';
        } else {
            optimalCost = data.Cost.Optimal.toLocaleString() + " gil";
        }

        if (data.Cost.Shop <= 0) {
            shopCost =  'UNAVAILABLE';
        } else {
            shopCost = data.Cost.Shop.toLocaleString() + " gil";
        }

        if (data.Cheap.LQ === null) {
            cheap =  'UNAVAILABLE';
        } else {
            cheap = data.Cheap.LQ.Item.PricePerUnit.toLocaleString()+" gil";
            cheap +=" ("+data.Cheap.LQ.Count+" listings)";
        }
        if (data.Cheap.HQ !== null) {
            cheap += " / <img src='hq.png'>";
            cheap += data.Cheap.HQ.Item.PricePerUnit.toLocaleString()+" gil";
            cheap +=" ("+data.Cheap.HQ.Count+" listings)";
        }

        if (data.Recent.LQ === null) {
            recent =  'UNAVAILABLE';
        } else {
            recent = data.Recent.LQ.PricePerUnit.toLocaleString()+" gil";
        }
        if (data.Recent.HQ !== null) {
            recent += " / <img src='hq.png'>";
            recent += data.Recent.HQ.PricePerUnit.toLocaleString()+" gil";
        }

        if (data.Week.LQ.Average === 0) {
            week =  'UNAVAILABLE';
        } else {
            week = data.Week.LQ.Average.toLocaleString()+" gil";
            sales = data.Week.LQ.Count;
            plu = 's';
            if (sales === 1) {
                plu = '';
            }
            week +=" ( "+sales.toLocaleString()+" sale"+plu+" )";
        }
        if (data.Week.HQ.Average !== 0) {
            week += " / <img src='hq.png'>";
            week += data.Week.HQ.Average.toLocaleString()+" gil";
            sales = data.Week.HQ.Count;
            plu = 's';
            if (sales === 1) {
                plu = '';
            }
            week +=" ( "+sales.toLocaleString()+" sale"+plu+" )";
        }

        var server = encodeURI(document.getElementById('server').value);
        var item = encodeURI(data.ID);
        if (data.Info.Result.Amount > 1) {
            dataName = '<a href="index.php?server='+server+'&item='+item+'" target="_blank">';
            dataName += data.Name + ' x' + data.Info.Result.Amount + '</a>';
        } else {
            dataName = '<a href="index.php?server='+server+'&item='+item+'" target="_blank">';
            dataName += data.Name + "</a>";
        }

        var sect =
        '<div style="border: 5px solid gray">'+
        '<h2 style="text-align:center;">'+dataName+'</h2><hr>' +
        '<div>' +
        'Recent: '+recent+'<br>'+
        'Weekly Average: '+week+'<br>'+
        'Current: '+cheap+"<br>"+
        "<hr>" +
        'Vendor Cost: '+shopCost+"<br>" +
        'Craft at Market Cost: '+marketCost+'<br>'+
        'Craft at Optimal Cost: '+optimalCost+"<br>";
        if (data.Profit.LQ > 0 || data.Profit.HQ > 0) {
            var block = '<hr>';
            if (data.Profit.LQ > 0) {
                block += '<b>Possible Profit</b>: '+data.Profit.LQ.toLocaleString()+" gil ("+Math.round(data.Profit["LQ%"]*100)+"%)<br>";
            }
            if (data.Profit.HQ > 0) {
                block += "<b>Possible Profit</b>: <img src='hq.png'>"+data.Profit.HQ.toLocaleString()+" gil ("+Math.round(data.Profit["HQ%"]*100)+"%)<br>";
            }
            block += "<hr>";
            sect += block;
        }
        sect += "</div></div>";
        document.getElementById('output').innerHTML += sect;
    });
}

function getDataBlock() {
  document.getElementById('output').innerHTML = "";
  var xhttp = new XMLHttpRequest();
  var server = encodeURI(document.getElementById('server').value);
  var book = encodeURI(document.getElementById('book').value);
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        hideSpinner();
        var data = JSON.parse(this.responseText);
        fillBookFrame(data);
    }
  };
  showSpinner();
  xhttp.open('POST', 'html_master.php', true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send('book=' + book +'&server=' + server);
}

function getDataEvent() {
  showSpinner();
  document.getElementById('progress').value = 0;
  document.getElementById('progress').style.display = "block";
  var server = encodeURI(document.getElementById('server').value);
  var book = encodeURI(document.getElementById('book').value);
  var source = new EventSource("html_master.php?event=1&book=" + book +
        "&server=" + server);
  source.onmessage = function(event) {
      var data = JSON.parse(event.data);
      if (data.type == "start") {
        document.getElementById('progress').max = data.data;
      } else if (data.type == "progress") {
        document.getElementById('progress').value += 1;
      } else if (data.type == "info") {
        console.log(data.data);
      } else if (data.type == "done") {
        source.close();
        hideSpinner();
        document.getElementById('progress').style.display = "none";
        var book = JSON.parse(data.data);
        fillBookFrame(book);
      }
  };
}

function getData() {
    document.getElementById('output').innerHTML = "";
    if(typeof(EventSource) !== "undefined") {
      getDataEvent();
    } else {
      getDataBlock();
    }
}
