/* jshint browser: true */

function fillShortFrame(dataset)
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
            dataName = '<a href="index.php?server='+server+'&item='+item +
             '&crafter='+data.Info.CraftTypeName+'" target="_blank">';
            dataName += data.Name + ' x' + data.Info.Result.Amount + '</a>';
        } else {
            dataName = '<a href="index.php?server='+server+'&item='+item+
                '&crafter='+data.Info.CraftTypeName+'" target="_blank">';
            dataName += data.Name + "</a>";
        }

        subText = data.Info.CraftTypeName + ' - lvl ' +
                  data.Info.RecipeLevel.ClassJobLevel;
        if (data.Info.Book !== null) {
            subText += " from '<a href=\"masterbook.php?server=" + server +
                "&item="+ encodeURIComponent(data.Info.Book.Name) +
                "\" target='_blank'>" + data.Info.Book.Name + "</a>'";
        }

        var sect =
        '<div class="card m-5 shadow-lg">'+
        '<div class="card-body">'+
        '<h2 style="text-align:center;">'+dataName+'</h2><hr>' +
        '<h4 class="text-center">'+subText+'</h4>' +
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
            sect += block;
        }
        sect += "</div></div></div>";
        document.getElementById('output').innerHTML += sect;
    });
}
