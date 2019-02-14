/* jshint browser: true */

function getBadge(order, value) {
    var i = order.indexOf(value);
    if (order.length == 3 && i == 2) {
        return '<span class="badge badge-danger">';
    } else if ( order.length >= 2 && i == 1) {
        return '<span class="badge badge-primary">';
    } else {
        return '<span class="badge badge-success">';
    }
}

function printLine(line, tab)
{
    var server = document.getElementById('server').value;
    var output = '<ul class="list-style">\n';
    line.forEach(function (l) {
        output += '<li>\n';
        if (l.craftCost > 0) {
            output += '<a href="index.php?server='+server+'&item='+l.id+
            '&crafter='+l.craftedBy+'" target="_blank">';
        } else {
            output += '<a href="https://www.ffxivmb.com/Items/'+server+'/'+l.id+'" target="_blank">';
        }
       output += l.name+'</a> (x'+l.count+') -> ';

        var low = [];
        if (l.marketCost > 0) low.push(l.marketCost);
        if (l.craftCost > 0) low.push(l.craftCost);
        if (l.shopCost > 0) low.push(l.shopCost);
        low.sort(function(a, b){return a - b;});

        if (l.marketCost > 0) {
            output += getBadge(low, l.marketCost);
            output += 'market</span>&nbsp;';
            output += l.marketCost.toLocaleString() + " gil ";
            if (l.marketHQ) {
                output += "<img src='hq.png'>";
            }
        } else {
            output += "UNAVAILABLE";
        }
        if (l.craftCost > 0) {
            output += '&nbsp;' + getBadge(low, l.craftCost);
            output += 'crafted</span>&nbsp(' +
              '<a href="index.php?server='+server+'&item='+l.id +
              '&crafter='+l.craftedBy+'" target="_blank">'+ l.craftedBy +
              '</a>) ' + l.craftCost.toLocaleString() + " gil";
        }
        if (l.shopCost > 0) {
            output += '&nbsp;' + getBadge(low, l.shopCost);
            output += 'vendor</span>&nbsp;';
            output +='<a href="http://www.garlandtools.org/db/#item/' +
                    l.id + '" target="_blank">' +
                    l.shopCost.toLocaleString() + "</a> gil";
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

    dataName ='<a href="http://www.garlandtools.org/db/#item/' +
        data.ID + '" target="_blank">' + data.Name + "</a>";
    if (data.Info !== null && data.Info.Result.Amount > 1) {
        dataName += ' x' + data.Info.Result.Amount;
    }

    subText = '';
    if (data.Info !== null) {
        subText = data.Info.CraftTypeName + ' - lvl ' +
                  data.Info.RecipeLevel.ClassJobLevel;
        if (data.Info.Book !== null) {
            var server = document.getElementById('server').value;
            subText += " from '<a href=\"masterbook.php?server=" + server +
                "&item="+ encodeURIComponent(data.Info.Book.Name) +
                "\" target='_blank'>" + data.Info.Book.Name + "</a>'";
        }
    }

    var content =
    '<div class="card m-5">'+
    '  <div class="card-body">'+
    '    <h2 class="card-title text-center">'+dataName+'</h2>' +
    '    <h4 class="text-center">'+subText+'</h4>' +
    '    <ul class="list-unstyled text-left">' +
    '       <li>Recent: '+recent+'</li>'+
    '       <li>Weekly Average: '+week+'</li>'+
    '       <li>Current: '+cheap+"</li>"+
    '    </ul>' +
    '  <ul class="list-unstyled text-left">' +
    '    <li>Vendor Cost: '+shopCost+"</li>" +
    '    <li>Craft at Market Cost: '+marketCost+'</li>'+
    '    <li>Craft at Optimal Cost: '+optimalCost+"</li>"+
    '  </ul>';
    if (data.Profit.LQ > 0 || data.Profit.HQ > 0) {
        content += '<ul class="list-unstyled text-left">';
        if (data.Profit.LQ > 0) {
            content += '<li><b>Possible Profit</b>: '+data.Profit.LQ.toLocaleString()+" gil ("+Math.round(data.Profit["LQ%"]*100)+"%)</li>";
        }
        if (data.Profit.HQ > 0) {
            content += "<li><b>Possible Profit</b>: <img src='hq.png'>"+data.Profit.HQ.toLocaleString()+" gil ("+Math.round(data.Profit["HQ%"]*100)+"%)</lli>";
        }
        content += "</ul>\n";
    }
    content += "<div class='card text-left'>\n";
    content += printLine(data.Recipe, 0);
    content += "</div>";
    content += "</div>\n";

    document.getElementById('output').innerHTML  = content;
}

function getDataBlock() {
  document.getElementById('output').innerHTML = "";
  var xhttp = new XMLHttpRequest();
  var server = encodeURI(document.getElementById('server').value);
  var item = encodeURI(document.getElementById('item').value);
  var crafter = encodeURI(document.getElementById('crafter').value);
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        $("#refresh_spinner").modal("hide");
        var data = JSON.parse(this.responseText);
        fillCraftFrame(data);
    }
  };
  $("#refresh_spinner").modal({backdrop: 'static', keyboard: false});
  xhttp.open('POST', 'html_craft.php', true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send('item=' + item +'&server=' + server + '&crafter=' + crafter);
}

function getDataEvent() {
  $("#refresh_spinner").modal({backdrop: 'static', keyboard: false});
  document.getElementById('progress_bar').style.width = "1%";
  document.getElementById('progress_bar').setAttribute('aria-valuenow', 0);
  document.getElementById('progress').style.display = '';
  var server = encodeURI(document.getElementById('server').value);
  var item = encodeURI(document.getElementById('item').value);
  var crafter = encodeURI(document.getElementById('crafter').value);
  var source = new EventSource("html_craft.php?event=1&item=" + item +
        "&server=" + server + "&crafter=" + crafter);
  source.onmessage = function(event) {
      var data = JSON.parse(event.data);
      if (data.type == "start") {
        document.getElementById('progress_bar').setAttribute('aria-valuemax', data.data);
        document.getElementById('progress_bar').setAttribute('aria-valuenow', 0);
      } else if (data.type == "progress") {
        var w = document.getElementById('progress_bar').getAttribute('aria-valuenow');
        w = parseInt(w)+1;
        document.getElementById('progress_bar').setAttribute('aria-valuenow', w);
        var p = Math.round((w / document.getElementById('progress_bar').getAttribute('aria-valuemax')) * 100);
        p = p + '%';
        document.getElementById('progress_bar').style.width = p;
      } else if (data.type == "info") {
        console.log(data.data);
      } else if (data.type == "done") {
        source.close();
        $("#refresh_spinner").modal("hide");
        document.getElementById('progress').style.display = "none";
        if (data.data === "[]") {
            var name = document.getElementById('item').value;
            alert("Failed to find recipe for item  '" + name + "'");
        }
        var item = JSON.parse(data.data);
        fillCraftFrame(item);
      }
  };
}

function getData() {
    if(typeof(EventSource) !== "undefined") {
      getDataEvent();
    } else {
      getDataBlock();
    }
}
