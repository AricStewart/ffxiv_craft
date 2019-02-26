<?php
/**
 * Access to datafiles from https://github.com/viion/ffxiv-datamining
 *
 * PHP version 5
 *
 * @category  Final_Fantasy_XIV
 * @package   FfxivDataSet
 * @author    Aric Stewart <aricstewart@google.com>
 * @copyright 2019 Aric Stewart
 * @license   Apache License, Version 2.0
 * @link      <none>
 **/

/*.
    require_module 'array';
    require_module 'core';
    require_module 'file';
    require_module 'phpinfo';
    require_module 'json';
e*/

require_once __DIR__.'/db.inc';
require __DIR__.'/vendor/autoload.php';
use GuzzleHttp\Client;

ini_set('memory_limit', '5000M');

function _nameSort($a, $b) 
{
    return strnatcmp($a->Name, $b->Name);
}

class FfxivDataSet
{
    public $gather = array();
    public $world = array();
    public $gilShop = array();
    public $master = array();

    /* Company Data */
    private $_companySequence = array();

    private $_RecipeLevelTable = array();

    private $_recipData = "/data/Recipe.csv";
    private $_recipURL = "Recipe.csv";

    private $_itemData = "/data/Item.csv";
    private $_itemURL =  "Item.csv";

    private $_gatherData = "/data/GatheringItem.csv";
    private $_gatherURL = "GatheringItem.csv";

    private $_masterData = "/data/SecretRecipeBook.csv";
    private $_masterURL = "SecretRecipeBook.csv";

    private $_worldData = "/data/World.csv";
    private $_worldURL = "World.csv";

    private $_gilShopData = "/data/GilShopItem.csv";
    private $_gilShopURL = "GilShopItem.csv";

    private $_RecipeLevelTableData = "/data/RecipeLevelTable.csv";
    private $_RecipeLevelTableURL = "RecipeLevelTable.csv";

    private $_CompanyCraftSequence = "/data/CompanyCraftSequence.csv";
    private $_CompanyCraftSequenceURL = "CompanyCraftSequence.csv";
    private $_CompanyCraftPart = "/data/CompanyCraftPart.csv";
    private $_CompanyCraftPartURL = "CompanyCraftPart.csv";
    private $_CompanyCraftProcess = "/data/CompanyCraftProcess.csv";
    private $_CompanyCraftProcessURL = "CompanyCraftProcess.csv";
    private $_CompanyCraftSupplyItem = "/data/CompanyCraftSupplyItem.csv";
    private $_CompanyCraftSupplyItemURL = "CompanyCraftSupplyItem.csv";

    private $_client;
    public $baseData;

    public $craftType = [
            "Woodworking",
            "Smithing",
            "Armorcraft",
            "Goldsmithing",
            "Leatherworking",
            "Clothcraft",
            "Alchemy",
            "Cooking",
        ];

    private static function _isTrue($val)
    {
        $boolval = ( is_string($val) ? filter_var(
            $val,
            FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE
        ) : (bool) $val );
        return ( $boolval===null ? false : $boolval );
    }

    private static function _filterIngredient($item) 
    {
        return ($item['Item'] > 0 && $item['Amount'] > 0);
    }

    private static function _filterBaseparam($item) 
    {
        return ($item['Param'] > 0 && $item['Value'] > 0);
    }

    private static function _filterNull($item) 
    {
        return ($item != NULL);
    }

    private static function _filterCompanySet($item) 
    {
        return ($item['Quantity'] > 0);
    }

    private function _getData($file, $url) 
    {
        $response = $this->_client->request('GET', $url);
        @file_put_contents($file, $response->getBody());
        @chmod($file, 0666);
    }

    private function _loadItems($force=false) 
    {
        if ($force || !file_exists($this->baseData.$this->_itemData)) {
            $this->_getData($this->baseData.$this->_itemData, $this->_itemURL);
            $force=true;
        }
        if (!$force) {
            $sql = "SELECT COUNT(`Id`) AS total FROM `items` WHERE 1;";
            $result = DB::run($sql);
            $value = $result->fetch();
            if ($value !== false && $value['total'] > 0) {
                return;
            }
        }

        $sql = "DELETE FROM `items` WHERE 1";
        DB::run($sql);
        $handle = fopen($this->baseData.$this->_itemData, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 2000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if ($data[1] === "") {
                    continue;
                }
                $entry = [
                    'Index' => intval($data[0]),
                    'Singular' => $data[1],
                    'Adjective' => intval($data[2]),
                    'Plural' => $data[3],
                    'PossessivePronoun' => intval($data[4]),
                    'StartsWithVowel' => intval($data[5]),
                    'Pronoun' => intval($data[7]),
                    'Article' => intval($data[8]),
                    'Description' => $data[9],
                    'Name' => $data[10],
                    'Icon' => intval($data[11]),
                    'Level{Item}' => intval($data[12]),
                    'Rarity' => intval($data[13]),
                    'FilterGroup' => intval($data[14]),
                    'AdditionalData' => intval($data[15]),
                    'ItemUICategory' => intval($data[16]),
                    'ItemSearchCategory' => intval($data[17]),
                    'EquipSlotCategory' => intval($data[18]),
                    'StackSize' => intval($data[20]),
                    'IsUnique' => $this->_isTrue($data[21]),
                    'IsUntradable' => $this->_isTrue($data[22]),
                    'IsIndisposable' => $this->_isTrue($data[23]),
                    'IsEquippable' => $this->_isTrue($data[24]),
                    'PriceMid' => intval($data[25]),
                    'PriceLow' => intval($data[26]),
                    'CanBeHq' => $this->_isTrue($data[27]),
                    'IsDyeable' => $this->_isTrue($data[28]),
                    'IsCrestWorthy' => $this->_isTrue($data[29]),
                    'ItemAction' => intval($data[30]),
                    'Cooldown<s>' => intval($data[32]),
                    'ClassJob{Repair}' => intval($data[33]),
                    'Item{Repair}' => intval($data[34]),
                    'Item{Glamour}' => intval($data[35]),
                    'Salvage' => intval($data[36]),
                    'IsCollectable' => $this->_isTrue($data[37]),
                    'AetherialReduce' => intval($data[38]),
                    'Level{Equip}' => intval($data[39]),
                    'EquipRestriction' => intval($data[41]),
                    'ClassJobCategory' => intval($data[42]),
                    'GrandCompany' => intval($data[43]),
                    'ItemSeries' => intval($data[44]),
                    'BaseParamModifier' => intval($data[45]),
                    'Model{Main}' => $data[46],
                    'Model{Sub}' => $data[47],
                    'ClassJob{Use}' => intval($data[48]),
                    'Damage{Phys}' => intval($data[50]),
                    'Damage{Mag}' => intval($data[51]),
                    'Delay<ms>' => intval($data[52]),
                    'BlockRate' => intval($data[54]),
                    'Block' => intval($data[55]),
                    'Defense{Phys}' => intval($data[56]),
                    'Defense{Mag}' => intval($data[57]),
                    'BaseParam' => [
                        ['Param' => intval($data[58]),
                         'Value' => intval($data[59])],
                        ['Param' => intval($data[60]),
                         'Value' => intval($data[61])],
                        ['Param' => intval($data[62]),
                         'Value' => intval($data[63])],
                        ['Param' => intval($data[64]),
                         'Value' => intval($data[65])],
                        ['Param' => intval($data[66]),
                         'Value' => intval($data[67])],
                        ['Param' => intval($data[68]),
                         'Value' => intval($data[69])]],
                    'ItemSpecialBonus' => intval($data[70]),
                    'ItemSpecialBonus{Param}' => intval($data[71]),
                    'BaseParam{Special}' => [
                        ['Param' => intval($data[72]),
                         'Value' => intval($data[73])],
                        ['Param' => intval($data[74]),
                         'Value' => intval($data[75])],
                        ['Param' => intval($data[76]),
                         'Value' => intval($data[77])],
                        ['Param' => intval($data[78]),
                         'Value' => intval($data[79])],
                        ['Param' => intval($data[80]),
                         'Value' => intval($data[81])],
                        ['Param' => intval($data[82]),
                         'Value' => intval($data[83])]],
                    'MaterializeType' => intval($data[84]),
                    'MateriaSlotCount' => intval($data[85]),
                    'IsAdvancedMeldingPermitted' => $this->_isTrue($data[86]),
                    'IsPvP' => $this->_isTrue($data[87]),
                    'IsGlamourous' => $this->_isTrue($data[89])
                ];
                $entry['BaseParam'] = array_filter(
                    $entry['BaseParam'],
                    'FfxivDataSet::_filterBaseparam'
                );
                $entry['BaseParam{Special}'] = array_filter(
                    $entry['BaseParam{Special}'],
                    'FfxivDataSet::_filterBaseparam'
                );

                $id = $entry['Index'];
                $name = myPDO::quote($entry['Name']);
                $data = myPDO::quote(json_encode($entry));

                $sql = "INSERT INTO `items` (Id, Name, Data) ".
                       "VALUES ($id, $name, $data); ";
                DB::run($sql);
            }
            fclose($handle);
        }
    }

    private function _checkItemTable() 
    {
        $sql = "SHOW TABLES LIKE 'items';";
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value === false) {
            $sql = <<<SQL
                CREATE TABLE items (
                    Id INT UNSIGNED NOT NULL PRIMARY KEY,
                    Name TEXT NOT NULL,
                    Data TEXT NOT NULL,
                    Date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
                );
SQL;
            DB::run($sql);
        }
        $this->_loadItems();
    }

    private function _loadRecipes($force=false)
    {
        if ($force || !file_exists($this->baseData.$this->_recipData)) {
            $this->_getData($this->baseData.$this->_recipData, $this->_recipURL);
            $force=true;
        }
        if (!$force) {
            $sql = "SELECT COUNT(`Id`) AS total FROM `recipes` WHERE 1;";
            $result = DB::run($sql);
            $value = $result->fetch();
            if ($value !== false && $value['total'] > 0) {
                return;
            }
        }

        $this->_loadRecipeLevelTable();

        $sql = "DELETE FROM `recipes` WHERE 1";
        DB::run($sql);

        $handle = fopen($this->baseData.$this->_recipData, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if ($data[0] === '0' || $data[1] === '0') {
                    continue;
                }
                $entry = ['Index' => intval($data[0]),
                    'CraftType' => intval($data[2]),
                    'RecipeLevelTable' => intval($data[3]),
                    'Result' => [
                        'Item' =>intval($data[4]),
                        'Amount' => intval($data[5])],
                    'Ingredients' => [
                        ['Item' => intval($data[6]),
                         'Amount' => intval($data[7])],
                        ['Item' => intval($data[8]),
                         'Amount' => intval($data[9])],
                        ['Item' => intval($data[10]),
                         'Amount' => intval($data[11])],
                        ['Item' => intval($data[12]),
                         'Amount' => intval($data[13])],
                        ['Item' => intval($data[14]),
                         'Amount' => intval($data[15])],
                        ['Item' => intval($data[16]),
                         'Amount' => intval($data[17])],
                        ['Item' => intval($data[18]),
                         'Amount' => intval($data[19])],
                        ['Item' => intval($data[20]),
                         'Amount' => intval($data[21])],
                        ['Item' => intval($data[22]),
                        'Amount' => intval($data[23])],
                        ['Item' => intval($data[24]),
                         'Amount' => intval($data[25]),]],
                    'RecipeElement' => $data[26],
                    'IsSecondary' => $this->_isTrue($data[28]),
                    'DifficultyFactor' => $data[29],
                    'QualityFactor' => $data[30],
                    'DurabilityFactor' => $data[31],
                    'RequiredCraftsmanship' => $data[33],
                    'RequiredControl' => $data[34],
                    'QuickSynthCraftsmanship' => $data[35],
                    'QuickSynthControl' => $data[36],
                    'SecretRecipeBook' => $data[37],
                    'CanQuickSynth' => $this->_isTrue($data[38]),
                    'CanHq' => $this->_isTrue($data[39]),
                    'ExpRewarded' => $this->_isTrue($data[40]),
                    'Status{Required}' => $data[41],
                    'Item{Required}' => $data[42],
                    'IsSpecializationRequired' => $this->_isTrue($data[43]),
                    'PatchNumber' => $data[45]
                    ];
                $entry['Ingredients'] = array_filter(
                    $entry['Ingredients'],
                    'FfxivDataSet::_filterIngredient'
                );

                $id = $entry['Index'];
                $item = $entry['Result']['Item'];
                $type = $entry['CraftType'];
                if ($entry['SecretRecipeBook'] > 0) {
                    $book = $entry['SecretRecipeBook'];
                } else {
                    $book = 'NULL';
                }
                $data = myPDO::quote(json_encode($entry));
                $level = $this->_RecipeLevelTable[$entry['RecipeLevelTable']]->ClassJobLevel;

                $sql = "INSERT INTO `recipes` (Id, Item, CraftType,".
                       "RecipeLevel, Book, Data) ".
                       "VALUES ($id, $item, $type, $level, $book, $data); ";
                DB::run($sql);
            }
            fclose($handle);
        }
    }

    private function _checkRecipeTable() 
    {
        $sql = "SHOW TABLES LIKE 'recipes';";
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value === false) {
            $sql = <<<SQL
                CREATE TABLE recipes (
                    Id INT UNSIGNED NOT NULL PRIMARY KEY,
                    Item INT UNSIGNED NOT NULL,
                    CraftType INT NOT NULL,
                    RecipeLevel INT NOT NULL,
                    Book INT,
                    Data TEXT NOT NULL,
                    Date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
                );
SQL;
            DB::run($sql);
        }
        $this->_loadRecipes();
    }

    function __construct($baseData='.') 
    {
        $this->baseData = $baseData;
        $this->_client = new Client(
            ['base_uri' =>
            "https://raw.githubusercontent.com/viion/ffxiv-datamining/master/csv/"]
        );
    }


    function loadGathering() 
    {
        if (count($this->gather) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->_gatherData)) {
            $this->_getData($this->baseData.$this->_gatherData, $this->_gatherURL);
        }
        $handle = fopen($this->baseData.$this->_gatherData, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if ($data[1] === "0") {
                    continue;
                }
                $entry = [
                    'Index' => intval($data[0]),
                    'Item' => intval($data[1]),
                    'GatheringItemLevel' => intval($data[2]),
                    'IsHidden' => $this->_isTrue($data[3])
                ];

                $this->gather[$data[1]] = $entry;
            }
            fclose($handle);
        }
    }

    function loadMasterBooks() 
    {
        if (count($this->master) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->_masterData)) {
            $this->_getData($this->baseData.$this->_masterData, $this->_masterURL);
        }
        $handle = fopen($this->baseData.$this->_masterData, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if ($data[1] === "0") {
                    continue;
                }
                $entry = [
                    'Index' => intval($data[0]),
                    'Item' => intval($data[1]),
                    'Name' => $data[2],
                ];
                $this->master[] = json_decode(json_encode($entry));
            }
            usort($this->master, "_nameSort");
            fclose($handle);
        }
    }

    function loadWorld() 
    {
        if (count($this->world) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->_worldData)) {
            $this->_getData($this->baseData.$this->_worldData, $this->_worldURL);
        }
        $handle = fopen($this->baseData.$this->_worldData, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if (!$this->_isTrue($data[4])) {
                    continue;
                }
                $entry = [
                    'Index' => intval($data[0]),
                    'Name' => $data[1],
                    'DataCenter' => intval($data[3]),
                    'Public' => $this->_isTrue($data[3]),
                ];
                $this->world[] = json_decode(json_encode($entry));
            }
            usort($this->world, "_nameSort");
            fclose($handle);
        }
    }

    function loadGilShop() 
    {
        if (count($this->gilShop) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->_gilShopData)) {
            $this->_getData($this->baseData.$this->_gilShopData, $this->_gilShopURL);
        }
        $handle = fopen($this->baseData.$this->_gilShopData, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                $entry = [
                    'Index' => $data[0],
                    'Item' => intval($data[1]),
                ];
                $this->gilShop[] = $entry;
            }
            fclose($handle);
        }
    }

    function findItemInShop($itemID) 
    {
        $this->loadGilShop();
        $result = [];
        foreach ($this->gilShop as $val) {
            if ($val['Item'] === $itemID) {
                $result[] = $val;
            }
        }
        return $result;
    }

    private function _loadRecipeLevelTable() 
    {
        if (count($this->_RecipeLevelTable) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->_RecipeLevelTableData)) {
            $this->_getData($this->baseData.$this->_RecipeLevelTableData, $this->_RecipeLevelTableURL);
        }
        $handle = fopen($this->baseData.$this->_RecipeLevelTableData, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                $entry = [
                    'ClassJobLevel' => intval($data[1]),
                    'Stars' => intval($data[2]),
                    'SuggestedCraftsmanship' => intval($data[3]),
                    'Difficulty' => intval($data[4]),
                    'Quality' => intval($data[5]),
                    'Durability' => intval($data[6]),
                ];
                $this->_RecipeLevelTable[$data[0]] = json_decode(
                    json_encode($entry));
            }
            fclose($handle);
        }
    }

    function getFullRecipe($id, $count, &$output, $subrun, $preference)
    {
        if ($output == null) {
            $output = array();
        }
        $tgt = $this->getRecipe($id, $preference);
        $ingred = &$output;
        if (!$subrun) {
            $output['Info'] = $tgt;
            $output['Count'] = $count;
            $output['Recipe'] = [];
            $ingred = &$output['Recipe'];
        }
        if ($tgt->Ingredients === null) {
            return $output;
        }
        foreach ($tgt->Ingredients as $i) {
            $bits = [];
            $part = $this->getRecipe($i->Item, $preference);
            $craftBy = null;
            if ($part !== null) {
                $result = $part->Result->Amount;
                $craftBy = $part->CraftTypeName;
                $this->getFullRecipe(
                    $i->Item,
                    ceil(($i->Amount* $count)/$result), $bits, true,
                    $preference
                );
            }
            $ingred[] = [
                         'id' => $i->Item,
                         'name' => '',
                         'count' => $i->Amount * $count,
                         'marketCost' => 0,
                         'marketHQ' => 0,
                         'craftCost' => 0,
                         'craftedBy' => $craftBy,
                         'shopCost' => 0,
                         'bits' => $bits
                        ];
        }
        return $output;
    }

    function lookupMasterBook($name) 
    {
        $this->loadMasterBooks();
        if (is_numeric($name)) {
            foreach ($this->master as $r) {
                if ($r->Index == $name) {
                    return $r;
                }
            }
        }
        foreach ($this->master as $r) {
            if (strcasecmp($r->Name, $name) == 0) {
                return $r->Index;
            }
        }
        return null;
    }

    function getMasterCraft($book=null) 
    {
        $output = [];
        if ($book !== null) {
            $theBook = $this->lookupMasterBook($book);
            if ($theBook === null) {
                print "Failed to find volume ".$book."\n";
                return null;
            }
            $book = $theBook;
        }

        if ($book !== null) {
            $sql = "SELECT * FROM `recipes` WHERE `Book` = $book";
        } else {
            $sql = "SELECT * FROM `recipes` WHERE `Book` IS NOT NULL";
        }
        $result = DB::run($sql);
        $value = $result->fetch();
        while ($value !== false) {
            $output[] = $value['Item'];
            $value = $result->fetch();
        }
        return $output;
    }

    function getMasterBookJob($name)
    {
        $craftMap = [
            "Carpenter" => "Woodworking",
            "Blacksmith" => "Smithing",
            "Armorer" => "Armorcraft",
            "Goldsmith" => "Goldsmithing",
            "Leatherworker" => "Leatherworking",
            "Weaver" => "Clothcraft",
            "Alchemist" => "Alchemy",
            "Culinarian" => "Cooking",
        ];
        $c = explode(" ",$name)[1];
        $crafter = trim($c, ':');
        return $craftMap[$crafter];
    }

    function getItem($param)
    {
        $this->_checkItemTable();
        if (is_numeric($param)) {
            $sql = "SELECT `Data` FROM `items` WHERE `ID` = $param;";
        } else {
            $sql = "SELECT `Data` FROM `items` WHERE lower(Name) = '$param';";
        }
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value !== false) {
            return json_decode($value['Data']);
        }
        return null;
    }

    function getItems($param)
    {
        $this->_checkItemTable();
        if (is_numeric($param)) {
            $sql = "SELECT `ID` FROM `items` WHERE `ID` = $param;";
        } else {
            $param = strtolower($param);
            $sql = "SELECT `ID` FROM `items` WHERE lower(Name) LIKE '%$param%';";
        }
        $result = DB::run($sql);
        $value = $result->fetch();
        $output = null;
        while ($value !== false) {
            if ($output === null) {
                $output = [];
            }
            $output[] = $value['ID'];
            $value = $result->fetch();
        }
        return $output;
    }

    function _processRecipe($value, &$AllCraftTypes) {
        $data = json_decode($value['Data']);
        $data->CraftTypeName = $this->craftType[$data->CraftType];
        $AllCraftTypes[] = $data->CraftTypeName;
        $data->RecipeLevel = $this->_RecipeLevelTable[$data->RecipeLevelTable];
        if ($data->SecretRecipeBook > 0) {
            $this->loadMasterBooks();
            $data->Book = $this->lookupMasterBook($data->SecretRecipeBook);
        } else {
            $data->Book = null;
        }
        return $data;
    }

    function getRecipe($param, $preference)
    {
        $this->_loadRecipeLevelTable();
        $this->_checkRecipeTable();
        $sql = "SELECT `Data` FROM `recipes` WHERE `item` = $param;";
        $result = DB::run($sql);
        $value = $result->fetch();
        $output = null;
        $AllCraftTypes = [];
        while ($value !== false) {
            if ($output === null) {
                $output = [];
            }
            $output[] = $this->_processRecipe($value, $AllCraftTypes);
            $value = $result->fetch();
        }
        if ($output && $preference) {
            foreach($output as $data) {
                if (strcasecmp($data->CraftTypeName, $preference)==0) {
                    $data->AllCraftTypes = $AllCraftTypes;
                    return $data;
                }
            }
            $data = $output[0];
            $data->AllCraftTypes = $AllCraftTypes;
            return $data;
        }
        return $output;
    }

    function getRecipeSet($crafter, $levelLow=0, $levelHigh=0)
    {
        $this->_loadRecipeLevelTable();
        $this->_checkRecipeTable();
        if (!is_numeric($crafter)) {
            $crafter = array_search($crafter, $this->craftType);
            if ($crafter === false) {
                return null;
            }
        }
        $sql = "SELECT `Item` FROM `recipes` WHERE `CraftType` = $crafter";
        if ($levelLow > 0) {
            $sql .= " AND `RecipeLevel` >= $levelLow";
        }
        if ($levelHigh > 0) {
            $sql .= " AND `RecipeLevel` <= $levelHigh";
        }
        $result = DB::run($sql);
        $value = $result->fetch();
        $output = null;
        while ($value !== false) {
            $item = $this->getItem($value['Item']);
            if ($item->IsUntradable || $item->IsIndisposable) {
                $value = $result->fetch();
                continue;
            }
            if ($output === null) {
                $output = [];
            }
            $output[] = $value['Item'];
            $value = $result->fetch();
        }
        return $output;
    }

    /* Company Crafting Items */
    /*  CompanyCraftSequence.ResultItem Look at  CompanyCraftPart  (0-7) that
        references  CompanyCraftPart.key there are 3 CompanyCraftProcess
        values one for each stage. So off to  CompanyCraftProcess.csv  which
        spells out SupplyItem, SetQuantity and SetsRequired (1-11) ,
        SupplyItem is found in CompanyCraftSupplyItem, which gives and
        ItemID! Finally */

    private function _loadCompanyCraftSupplyItem() 
    {
        $table = array();

        if (!file_exists($this->baseData.$this->_CompanyCraftSupplyItem)) {
            $this->_getData($this->baseData.$this->_CompanyCraftSupplyItem, $this->_CompanyCraftSupplyItemURL);
        }
        $handle = fopen($this->baseData.$this->_CompanyCraftSupplyItem, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if ($data[1] === "0") {
                    continue;
                }
                $entry = [
                    'Index' => intval($data[0]),
                    'Item' => intval($data[1])
                ];
                $table[intval($data[0])] = json_decode(json_encode($entry));
            }
            fclose($handle);
        }
        return $table;
    }

    private function _loadCompanyCraftProcess($item)
    {
        $table = array();

        if (!file_exists($this->baseData.$this->_CompanyCraftProcess)) {
            $this->_getData($this->baseData.$this->_CompanyCraftProcess, $this->_CompanyCraftProcessURL);
        }
        $handle = fopen($this->baseData.$this->_CompanyCraftProcess, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if ($data[1] === "0") {
                    continue;
                }
                $entry = [
                    'Index' => intval($data[0]),
                    'Set' => [
                        ['Item' => $item[intval($data[1])],
                         'Quantity' => intval($data[2]),
                         'Required' => intval($data[3])],
                        ['Item' => $item[intval($data[4])],
                         'Quantity' => intval($data[5]),
                         'Required' => intval($data[6])],
                        ['Item' => $item[intval($data[7])],
                         'Quantity' => intval($data[8]),
                         'Required' => intval($data[9])],
                        ['Item' => $item[intval($data[10])],
                         'Quantity' => intval($data[11]),
                         'Required' => intval($data[12])],
                        ['Item' => $item[intval($data[13])],
                         'Quantity' => intval($data[14]),
                         'Required' => intval($data[15])],
                        ['Item' => $item[intval($data[16])],
                         'Quantity' => intval($data[17]),
                         'Required' => intval($data[18])],
                        ['Item' => $item[intval($data[19])],
                         'Quantity' => intval($data[20]),
                         'Required' => intval($data[21])],
                        ['Item' => $item[intval($data[22])],
                         'Quantity' => intval($data[23]),
                         'Required' => intval($data[24])],
                        ['Item' => $item[intval($data[25])],
                         'Quantity' => intval($data[26]),
                         'Required' => intval($data[27])],
                        ['Item' => $item[intval($data[28])],
                         'Quantity' => intval($data[29]),
                         'Required' => intval($data[30])],
                        ['Item' => $item[intval($data[31])],
                         'Quantity' => intval($data[32]),
                         'Required' => intval($data[33])],
                        ['Item' => $item[intval($data[34])],
                         'Quantity' => intval($data[35]),
                         'Required' => intval($data[36])],
                     ],
                ];

                $entry['Set'] = array_filter(
                    $entry['Set'],
                    'FfxivDataSet::_filterCompanySet'
                );

                $table[intval($data[0])] = json_decode(json_encode($entry));
            }
            fclose($handle);
        }
        return $table;
    }

    private function _loadCompanyCraftPart($process)
    {
        $table = array();

        if (!file_exists($this->baseData.$this->_CompanyCraftPart)) {
            $this->_getData($this->baseData.$this->_CompanyCraftPart, $this->_CompanyCraftPartURL);
        }
        $handle = fopen($this->baseData.$this->_CompanyCraftPart, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if ($data[1] === "0") {
                    continue;
                }
                $entry = [
                    'Index' => intval($data[0]),
                    'CompanyCraftType' => intval($data[2]),
                    'CompanyCraftProcess' => [
                        $process[intval($data[3])],
                        $process[intval($data[4])],
                        $process[intval($data[5])],
                     ],
                ];

                $entry['CompanyCraftProcess'] = array_filter(
                    $entry['CompanyCraftProcess'],
                    'FfxivDataSet::_filterNull'
                );

                $table[intval($data[0])] = json_decode(json_encode($entry));
            }
            fclose($handle);
        }
        return $table;
    }

    private function _loadCompanySequence($parts) 
    {
        $table = array();

        if (!file_exists($this->baseData.$this->_CompanyCraftSequence)) {
            $this->_getData($this->baseData.$this->_CompanyCraftSequence, $this->_CompanyCraftSequenceURL);
        }
        $handle = fopen($this->baseData.$this->_CompanyCraftSequence, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if ($data[1] === "0") {
                    continue;
                }
                $entry = [
                    'Index' => intval($data[0]),
                    'ResultItem' => intval($data[1]),
                    'CompanyCraftDraftCategory' => intval($data[3]),
                    'CompanyCraftType' => intval($data[4]),
                    'CompanyCraftDraft' => intval($data[5]),
                    'CompanyCraftPart' => [
                        $parts[intval($data[6])],
                        $parts[intval($data[7])],
                        $parts[intval($data[8])],
                        $parts[intval($data[9])],
                        $parts[intval($data[10])],
                        $parts[intval($data[11])],
                        $parts[intval($data[12])]]
                ];
                $entry['CompanyCraftPart'] = array_filter(
                    $entry['CompanyCraftPart'],
                    'FfxivDataSet::_filterNull'
                );

                $table[intval($data[1])] = json_decode(json_encode($entry));
            }
            fclose($handle);
        }
        return $table;
    }

    function loadCompanyCrafting()
    {
        if (count($this->_companySequence) > 0) {
            return $this->_companySequence;
        }
        $SupplyItem  = $this->_loadCompanyCraftSupplyItem();
        $Process  = $this->_loadCompanyCraftProcess($SupplyItem);
        $Parts = $this->_loadCompanyCraftPart($Process);
        $this->_companySequence = $this->_loadCompanySequence($Parts);

        return $this->_companySequence;
    }

    function listCompanyItems()
    {
        $this->loadCompanyCrafting();
        $items = array();
        foreach($this->_companySequence as $sequence) {
            $items[] = $this->getItem($sequence->ResultItem);
        }
        usort($items, "_nameSort");
        return $items;
    }
}
