# Final Fantasy XIV Crafting/Gathering Companion

This is primarily an exploration of some of the various FFXIV APIs available and applied to the area of economic crafting and gathering. 

### APIs used by this project
First of all I have to give thanks to the various projects being made use of here.

* FFXIV Market <https://www.ffxivmb.com>
* XIVAPI <https://xivapi.com/>
* XIV-Datamining <https://github.com/viion/ffxiv-datamining>

The idea is to make use of all three of these projects to bring together a useful tool for crafters to figure out what to make in the most economical way possible.

### Why PHP?
The initial vision of this project was to have it be a back end for a web page. However because of the time latency involved in the various API calls it very quickly became a command line tool. It may be rewritten in the future to fully embrace its new role but for now it functions.

### Setup
To setup you will need to acquire API keys from FFXIV Market and XIVAPI. Both are easily done from the respective web sites. Then fill in those values in the respective areas in this file.

### How do I find that Item ID you talk about?

If you are on FFXIV Market <https://www.ffxivmb.com> then you can see it in the url for the item.  If you look at the Garland Database <https://garlandtools.org/db/#> then click on the gear on the item and look at the item #. 

### Usage

There are 2 main tools, without much in the way of usage statements. These can mostly be seen as templates to build your own tools but they also should serve as being usable for the average non-programmers also, so a quick explanation here.

### ffxiv_craft.php

*ffxiv_craft.php \<item\>*

- \<item\> can be either the database ID for the item or the full item name in quotes.  For example 
	- ./ffxiv_craft.php 23815
	- ./ffxiv_craft.php "rakshasa dogi of healing"

*ffxiv_craft.php -m \<Master Book\>*

- \<Master Book\> is either the name of the Recipe book that will have all the recipe iterated from.  For example
	- ./ffxiv_craft.php -m "Master Weaver II"

	
### ffxiv_gather.php

*ffxiv_gather.php*

- Enumerate every gathering item and determine profitability.

*ffxiv_gather.php -l \<limit\>*

- Limit the list to \<limit\> items 

*ffxiv_gather.php \<item\>*

- Only fetch and print information about the given item ID or item name. Examples:
	- ./ffxiv_gather.php 13753
	- ./ffxiv_gather.php "cloud cotton boll"
