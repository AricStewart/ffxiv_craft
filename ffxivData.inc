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
.*/

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
    private $companySequence = array();

    private $RecipeLevelTable = array();

    private $recipeData = "/data/Recipe.csv";

    private $recipeURL = "Recipe.csv";

    private $itemData = "/data/Item.csv";

    private $itemURL = "Item.csv";

    private $gatherData = "/data/GatheringItem.csv";

    private $gatherURL = "GatheringItem.csv";

    private $masterData = "/data/SecretRecipeBook.csv";

    private $masterURL = "SecretRecipeBook.csv";

    private $worldData = "/data/World.csv";

    private $worldURL = "World.csv";

    private $gilShopData = "/data/GilShopItem.csv";

    private $gilShopURL = "GilShopItem.csv";

    private $RecipeLevelTableData = "/data/RecipeLevelTable.csv";

    private $RecipeLevelTableURL = "RecipeLevelTable.csv";

    private $CompanyCraftSequence = "/data/CompanyCraftSequence.csv";

    private $CompanyCraftSequenceURL = "CompanyCraftSequence.csv";

    private $CompanyCraftType = "/data/CompanyCraftType.csv";

    private $CompanyCraftTypeURL = "CompanyCraftType.csv";

    private $CompanyCraftPart = "/data/CompanyCraftPart.csv";

    private $CompanyCraftPartURL = "CompanyCraftPart.csv";

    private $CompanyCraftProcess = "/data/CompanyCraftProcess.csv";

    private $CompanyCraftProcessURL = "CompanyCraftProcess.csv";

    private $CompanyCraftSupplyItem = "/data/CompanyCraftSupplyItem.csv";

    private $CompanyCraftSupplyItemURL = "CompanyCraftSupplyItem.csv";

    private $client;

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


    private static function isTrue($val)
    {
        $boolval = ( is_string($val) ? filter_var(
            $val,
            FILTER_VALIDATE_BOOLEAN,
            FILTER_NULL_ON_FAILURE
        ) : (bool) $val );
        return ( $boolval === null ? false : $boolval );

    }


    private static function filterIngredient($item)
    {
        return ($item['Item'] > 0 && $item['Amount'] > 0);

    }


    private static function filterBaseparam($item)
    {
        return ($item['Param'] > 0 && $item['Value'] > 0);

    }


    private static function filterNull($item)
    {
        return ($item != null);

    }


    private static function filterCompanySet($item)
    {
        return ($item['Quantity'] > 0);

    }


    private function getData($file, $url)
    {
        $response = $this->client->request('GET', $url);
        @file_put_contents($file, $response->getBody());
        @chmod($file, 0666);

    }


    private function loadItems($force = false)
    {
        if ($force || !file_exists($this->baseData.$this->itemData)) {
            $this->getData($this->baseData.$this->itemData, $this->itemURL);
            $force = true;
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
        $handle = fopen($this->baseData.$this->itemData, "r");
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
                'IsUnique' => $this->isTrue($data[21]),
                'IsUntradable' => $this->isTrue($data[22]),
                'IsIndisposable' => $this->isTrue($data[23]),
                'IsEquippable' => $this->isTrue($data[24]),
                'PriceMid' => intval($data[25]),
                'PriceLow' => intval($data[26]),
                'CanBeHq' => $this->isTrue($data[27]),
                'IsDyeable' => $this->isTrue($data[28]),
                'IsCrestWorthy' => $this->isTrue($data[29]),
                'ItemAction' => intval($data[30]),
                'Cooldown<s>' => intval($data[32]),
                'ClassJob{Repair}' => intval($data[33]),
                'Item{Repair}' => intval($data[34]),
                'Item{Glamour}' => intval($data[35]),
                'Salvage' => intval($data[36]),
                'IsCollectable' => $this->isTrue($data[37]),
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
                'IsAdvancedMeldingPermitted' => $this->isTrue($data[86]),
                'IsPvP' => $this->isTrue($data[87]),
                'IsGlamourous' => $this->isTrue($data[89])
                ];
                $entry['BaseParam'] = array_filter(
                    $entry['BaseParam'],
                    'FfxivDataSet::filterBaseparam'
                );
                $entry['BaseParam{Special}'] = array_filter(
                    $entry['BaseParam{Special}'],
                    'FfxivDataSet::filterBaseparam'
                );

                $id = $entry['Index'];
                $name = strip_tags($entry['Name']);
                $name = preg_replace('/[^A-Za-z0-9 \'\-]/', '', $name);
                $name = MyPDO::quote($name);
                $data = MyPDO::quote(json_encode($entry));

                $sql = "INSERT INTO `items` (Id, Name, Data) "."VALUES ($id, $name, $data); ";
                DB::run($sql);
            }
            fclose($handle);
        }

    }


    private function checkItemTable()
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
        $this->loadItems();

    }


    private function loadRecipes($force = false)
    {
        if ($force || !file_exists($this->baseData.$this->recipeData)) {
            $this->getData($this->baseData.$this->recipeData, $this->recipeURL);
            $force = true;
        }
        if (!$force) {
            $sql = "SELECT COUNT(`Id`) AS total FROM `recipes` WHERE 1;";
            $result = DB::run($sql);
            $value = $result->fetch();
            if ($value !== false && $value['total'] > 0) {
                return;
            }
        }

        $this->loadRecipeLevelTable();

        $sql = "DELETE FROM `recipes` WHERE 1";
        DB::run($sql);

        $handle = fopen($this->baseData.$this->recipeData, "r");
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
                'Item' => intval($data[4]),
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
                'IsSecondary' => $this->isTrue($data[28]),
                'DifficultyFactor' => $data[29],
                'QualityFactor' => $data[30],
                'DurabilityFactor' => $data[31],
                'RequiredCraftsmanship' => $data[33],
                'RequiredControl' => $data[34],
                'QuickSynthCraftsmanship' => $data[35],
                'QuickSynthControl' => $data[36],
                'SecretRecipeBook' => $data[37],
                'CanQuickSynth' => $this->isTrue($data[38]),
                'CanHq' => $this->isTrue($data[39]),
                'ExpRewarded' => $this->isTrue($data[40]),
                'Status{Required}' => $data[41],
                'Item{Required}' => $data[42],
                'IsSpecializationRequired' => $this->isTrue($data[43]),
                'PatchNumber' => $data[45]
                    ];
                $entry['Ingredients'] = array_filter(
                    $entry['Ingredients'],
                    'FfxivDataSet::filterIngredient'
                );

                $id = $entry['Index'];
                $item = $entry['Result']['Item'];
                $type = $entry['CraftType'];
                if ($entry['SecretRecipeBook'] > 0) {
                    $book = $entry['SecretRecipeBook'];
                } else {
                    $book = 'NULL';
                }
                $data = MyPDO::quote(json_encode($entry));
                $level = $this->RecipeLevelTable[$entry['RecipeLevelTable']]->ClassJobLevel;

                $sql = "INSERT INTO `recipes` (Id, Item, CraftType,"."RecipeLevel, Book, Data) "."VALUES ($id, $item, $type, $level, $book, $data); ";
                DB::run($sql);

                foreach ($entry['Ingredients'] as $ingred) {
                    if ($ingred['Item'] > 0) {
                        $i = strval($ingred['Item']);
                        $a = strval($ingred['Amount']);
                        $sql = "INSERT INTO `ingredients` (Id, Recipe, Item, Amount, Level) VALUES ($id, $item, $i, $a, $level);";
                        DB::run($sql);
                    }
                }
            }
            fclose($handle);
        }

    }


    private function checkRecipeTable()
    {
        $force = false;
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
        $sql = "SHOW TABLES LIKE 'ingredients';";
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value === false) {
            $sql = <<<SQL
                CREATE TABLE ingredients(
                    Idx INT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
                    Id INT UNSIGNED NOT NULL,
                    Recipe INT UNSIGNED NOT NULL,
                    Item INT UNSIGNED NOT NULL,
                    Amount INT UNSIGNED NOT NULL,
                    Level INT UNSIGNED NOT NULL,
                    Date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
                );
SQL;
            DB::run($sql);
            $force = true;
        }
        $this->loadRecipes($force);

    }


    public function __construct($baseData = '.')
    {
        $this->baseData = $baseData;
        $this->client = new Client(
            ['base_uri' => "https://raw.githubusercontent.com/viion/ffxiv-datamining/master/csv/"]
        );

    }


    public function loadGathering()
    {
        if (count($this->gather) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->gatherData)) {
            $this->getData($this->baseData.$this->gatherData, $this->gatherURL);
        }
        $handle = fopen($this->baseData.$this->gatherData, "r");
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
                'IsHidden' => $this->isTrue($data[3])
                ];

                $this->gather[$data[1]] = $entry;
            }
            fclose($handle);
        }

    }


    public function loadMasterBooks()
    {
        if (count($this->master) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->masterData)) {
            $this->getData($this->baseData.$this->masterData, $this->masterURL);
        }
        $handle = fopen($this->baseData.$this->masterData, "r");
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


    public function loadWorld()
    {
        if (count($this->world) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->worldData)) {
            $this->getData($this->baseData.$this->worldData, $this->worldURL);
        }
        $handle = fopen($this->baseData.$this->worldData, "r");
        $skip = 3;
        if (empty($handle) === false) {
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if (!$this->isTrue($data[4])) {
                    continue;
                }
                $entry = [
                'Index' => intval($data[0]),
                'Name' => $data[1],
                'DataCenter' => intval($data[3]),
                'Public' => $this->isTrue($data[3]),
                ];
                $this->world[] = json_decode(json_encode($entry));
            }
            usort($this->world, "_nameSort");
            fclose($handle);
        }

    }


    public function loadGilShop()
    {
        if (count($this->gilShop) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->gilShopData)) {
            $this->getData($this->baseData.$this->gilShopData, $this->gilShopURL);
        }
        $handle = fopen($this->baseData.$this->gilShopData, "r");
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


    public function findItemInShop($itemID)
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


    private function loadRecipeLevelTable()
    {
        if (count($this->RecipeLevelTable) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->RecipeLevelTableData)) {
            $this->getData($this->baseData.$this->RecipeLevelTableData, $this->RecipeLevelTableURL);
        }
        $handle = fopen($this->baseData.$this->RecipeLevelTableData, "r");
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
                $this->RecipeLevelTable[$data[0]] = json_decode(
                    json_encode($entry)
                );
            }
            fclose($handle);
        }

    }


    public function getFullRecipe($id, $count, &$output, $subrun, $preference)
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
                    round(($i->Amount * $count) / $result, 2),
                    $bits,
                    true,
                    $preference
                );
            }
            $ingred[] = [
            'id' => $i->Item,
            'name' => '',
            'count' => $i->Amount * $count,
            'marketCost' => 0,
            'lastUpdate' => null,
            'marketHQ' => 0,
            'listCost' => 0,
            'craftCost' => 0,
            'craftedBy' => $craftBy,
            'shopCost' => 0,
            'bits' => $bits
                        ];
        }
        return $output;

    }


    public function lookupMasterBook($name)
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


    public function getMasterCraft($book = null)
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


    public function getMasterBookJob($name)
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
        $c = explode(" ", $name)[1];
        $crafter = trim($c, ':');
        return $craftMap[$crafter];

    }


    public function getItem($param)
    {
        $this->checkItemTable();
        if (is_numeric($param)) {
            $sql = "SELECT `Data` FROM `items` WHERE `ID` = $param;";
        } else {
            $p = MyPDO::quote($param);
            $sql = "SELECT `Data` FROM `items` WHERE lower(Name) = $p;";
        }
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value !== false) {
            return json_decode($value['Data']);
        }
        return null;

    }


    public function getItems($param)
    {
        $this->checkItemTable();
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


    private function processRecipe($value, &$AllCraftTypes)
    {
        $data = json_decode($value['Data']);
        $data->CraftTypeName = $this->craftType[$data->CraftType];
        $AllCraftTypes[] = $data->CraftTypeName;
        $data->RecipeLevel = $this->RecipeLevelTable[$data->RecipeLevelTable];
        if ($data->SecretRecipeBook > 0) {
            $this->loadMasterBooks();
            $data->Book = $this->lookupMasterBook($data->SecretRecipeBook);
        } else {
            $data->Book = null;
        }
        return $data;

    }


    public function getRecipe($param, $preference)
    {
        $this->loadRecipeLevelTable();
        $this->checkRecipeTable();
        $sql = "SELECT `Data` FROM `recipes` WHERE `item` = $param;";
        $result = DB::run($sql);
        $value = $result->fetch();
        $output = null;
        $AllCraftTypes = [];
        while ($value !== false) {
            if ($output === null) {
                $output = [];
            }
            $output[] = $this->processRecipe($value, $AllCraftTypes);
            $value = $result->fetch();
        }
        if ($output && $preference) {
            foreach ($output as $data) {
                if (strcasecmp($data->CraftTypeName, $preference) == 0) {
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


    public function getRecipeSet($crafter, $levelLow = 0, $levelHigh = 0)
    {
        $this->loadRecipeLevelTable();
        $this->checkRecipeTable();
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

    private function loadCompanyCraftSupplyItem()
    {
        $table = array();

        if (!file_exists($this->baseData.$this->CompanyCraftSupplyItem)) {
            $this->getData($this->baseData.$this->CompanyCraftSupplyItem, $this->CompanyCraftSupplyItemURL);
        }
        $handle = fopen($this->baseData.$this->CompanyCraftSupplyItem, "r");
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


    private function loadCompanyCraftProcess($item)
    {
        $table = array();

        if (!file_exists($this->baseData.$this->CompanyCraftProcess)) {
            $this->getData($this->baseData.$this->CompanyCraftProcess, $this->CompanyCraftProcessURL);
        }
        $handle = fopen($this->baseData.$this->CompanyCraftProcess, "r");
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
                    'FfxivDataSet::filterCompanySet'
                );

                $table[intval($data[0])] = json_decode(json_encode($entry));
            }
            fclose($handle);
        }
        return $table;

    }


    private function loadCompanyCraftPart($process, $type)
    {
        $table = array();

        if (!file_exists($this->baseData.$this->CompanyCraftPart)) {
            $this->getData($this->baseData.$this->CompanyCraftPart, $this->CompanyCraftPartURL);
        }
        $handle = fopen($this->baseData.$this->CompanyCraftPart, "r");
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
                'CompanyCraftType' => $type[intval($data[2])]->Name,
                'CompanyCraftProcess' => [
                $process[intval($data[3])],
                $process[intval($data[4])],
                $process[intval($data[5])],
                     ],
                ];

                $entry['CompanyCraftProcess'] = array_filter(
                    $entry['CompanyCraftProcess'],
                    'FfxivDataSet::filterNull'
                );

                $table[intval($data[0])] = json_decode(json_encode($entry));
            }
            fclose($handle);
        }
        return $table;

    }


    private function loadCompanyCraftType()
    {
        $table = array();

        if (!file_exists($this->baseData.$this->CompanyCraftType)) {
            $this->getData($this->baseData.$this->CompanyCraftType, $this->CompanyCraftTypeURL);
        }
        $handle = fopen($this->baseData.$this->CompanyCraftType, "r");
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
                'Name' => $data[1]
                ];

                $table[intval($data[0])] = json_decode(json_encode($entry));
            }
            fclose($handle);
        }
        return $table;

    }


    private function loadCompanySequence($parts, $type)
    {
        $table = array();

        if (!file_exists($this->baseData.$this->CompanyCraftSequence)) {
            $this->getData($this->baseData.$this->CompanyCraftSequence, $this->CompanyCraftSequenceURL);
        }
        $handle = fopen($this->baseData.$this->CompanyCraftSequence, "r");
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
                'CompanyCraftType' => $type[intval($data[4])]->Name,
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
                    'FfxivDataSet::filterNull'
                );

                $table[intval($data[1])] = json_decode(json_encode($entry));
            }
            fclose($handle);
        }
        return $table;

    }


    public function loadCompanyCrafting()
    {
        if (count($this->companySequence) > 0) {
            return $this->companySequence;
        }
        $SupplyItem  = $this->loadCompanyCraftSupplyItem();
        $Process  = $this->loadCompanyCraftProcess($SupplyItem);
        $Type = $this->loadCompanyCraftType();
        $Parts = $this->loadCompanyCraftPart($Process, $Type);
        $this->companySequence = $this->loadCompanySequence($Parts, $Type);

        return $this->companySequence;

    }


    public function listCompanyItems()
    {
        $this->loadCompanyCrafting();
        $items = array();
        foreach ($this->companySequence as $sequence) {
            $items[] = $this->getItem($sequence->ResultItem);
        }
        usort($items, "_nameSort");
        return $items;

    }


    public function listRecipiesUsingItem($item)
    {
        $this->checkItemTable();
        $this->checkRecipeTable();
        $output = [];
        $sql = "SELECT * FROM `ingredients` WHERE Item = $item;";
        $result = DB::run($sql);
        $value = $result->fetch();
        while ($value !== false) {
            $output[] = $value;
            $value = $result->fetch();
        }
        return $output;

    }

    public function resetAndReload()
    {
        $sql = "DROP TABLE IF EXISTS `recipes`;";
        DB::run($sql);
        $sql = "DROP TABLE IF EXISTS `ingredients`;";
        DB::run($sql);
        $sql = "DROP TABLE IF EXISTS `items`;";
        DB::run($sql);

        $this->checkItemTable();
        $this->checkRecipeTable();
        $this->loadItems(true);
        $this->loadRecipes(true);

    }


    /* End */
}
