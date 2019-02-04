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

function printLine(line, tab)
{
    var server = document.getElementById('server').value;
    var output = '<ul>\n';
    line.forEach(function (l) {
        output += '<li>\n';
        output += '<a href="https://www.ffxivmb.com/Items/'+server+'/'+l.id+'" target="_blank">'+
            l.name+'</a> (x'+l.count+') -> ';
        if (l.marketCost > 0) {
            output += " market "+l.marketCost.toLocaleString() + " gil ";
            if (l.marketHQ) {
                output += "<img src='hq.png'>";
            }
        } else {
            output += "UNAVALIABLE";
        }
        if (l.craftCost > 0) {
            output += '/ crafted ' + l.craftCost.toLocaleString() + " gil";
        }
        if (l.bits.length > 0) {
            output += printLine(l.bits, tab+1);
        }
        output += "</li>\n";
    });
    output += '</ul>\n';
    return output;
}

function fillCraftFrame(data)
{
    var marketCost = "";
    var cheap = "";
    if (data.Cost.Market <= 0) {
        marketCost =  'UNAVLAIBLE';
    } else {
        marketCost = data.Cost.Market.toLocaleString() + " gil";
    }
    if (data.Cheap <= 0) {
        cheap =  'UNAVLAIBLE';
    } else {
        cheap = data.Cheap.PricePerUnit.toLocaleString()+" gil";
        if (data.Cheap.IsHQ) {
            cheap += "<img src='hq.png'>";
        }
    }
    if (data.Recent <= 0) {
        recent =  'UNAVLAIBLE';
    } else {
        recent = data.Recent.PricePerUnit.toLocaleString()+" gil";
        if (data.Recent.IsHQ > 0) {
            recent += "<img src='hq.png'>";
        }
    }
    document.getElementById('output').innerHTML = 
    '<h2 style="text-align:center;">'+data.Name+'</h2><hr>' +
    '<div>' +
    'Recent: '+recent+'<br>'+
    'Current: '+cheap+"<br>"+
    'Market Cost: '+marketCost+'<br>'+
    'Optimal Cost: '+data.Cost.Optimal.toLocaleString()+" gil<br>";
    var Price = 0;
    if (data.Recent) {
        Price = data.Recent.PricePerUnit;
        if (data.Cheap > 0) {
            Price = Math.min(data.Cheap.PricePerUnit, Price);
        }
    }
    if (data.Cost.Optimal < Price) {
        document.getElementById('output').innerHTML += '<hr>' +
        '<b>Possible Profit</b>: '+(Price - data.Cost.Optimal).toLocaleString()+" gil<br><hr>";
    }
    document.getElementById('output').innerHTML += "</div>\n";
    document.getElementById('output').innerHTML += "<div>\n";
    document.getElementById('output').innerHTML += printLine(data.Recipe, 0);
    document.getElementById('output').innerHTML += "</div>\n";
}

function getData() {
  document.getElementById('output').innerHTML = "";
  var xhttp = new XMLHttpRequest();
  var server = encodeURI(document.getElementById('server').value);
  var item = encodeURI(document.getElementById('item').value);
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        hideSpinner();
        var data = JSON.parse(this.responseText);
        fillCraftFrame(data);
    }
  };
  showSpinner();
  xhttp.open('POST', 'html_craft.php', true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send('item=' + item +'&server=' + server);

}
