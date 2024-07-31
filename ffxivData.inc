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

    private $recipiesLoaded = false;

    private $itemsLoaded = false;

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
                $this->itemsLoaded = true;
                return;
            }
        }

        $sql = "DELETE FROM `items` WHERE 1";
        DB::run($sql);
        $handle = fopen($this->baseData.$this->itemData, "r");
        $skip = 3;
        $max_batch = 1000;
        if (empty($handle) === false) {
            $batch = $max_batch;
            $sql = '';
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
                /* data[6] */
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
                'ItemSortCategory' => intval($data[19]),
                /* $data[20] */
                'StackSize' => intval($data[21]),
                'IsUnique' => $this->isTrue($data[22]),
                'IsUntradable' => $this->isTrue($data[23]),
                'IsIndisposable' => $this->isTrue($data[24]),
                'Lot' => $this->isTrue($data[25]),
                'PriceMid' => intval($data[26]),
                'PriceLow' => intval($data[27]),
                'CanBeHq' => $this->isTrue($data[28]),
                'IsDyeable' => $this->isTrue($data[29]),
                'IsCrestWorthy' => $this->isTrue($data[30]),
                'ItemAction' => intval($data[31]),
                'CastTime<s>' => $data[32],
                'Cooldown<s>' => intval($data[33]),
                'ClassJob{Repair}' => intval($data[34]),
                'Item{Repair}' => intval($data[35]),
                'Item{Glamour}' => intval($data[36]),
                'Salvage' => intval($data[37]),
                'IsCollectable' => $this->isTrue($data[38]),
                'AlwaysCollectable' => $this->isTrue($data[39]),
                'AetherialReduce' => intval($data[40]),
                'Level{Equip}' => intval($data[41]),
                /* $data[42] */
                'EquipRestriction' => intval($data[43]),
                'ClassJobCategory' => intval($data[44]),
                'GrandCompany' => intval($data[45]),
                'ItemSeries' => intval($data[46]),
                'BaseParamModifier' => intval($data[47]),
                'Model{Main}' => $data[48],
                'Model{Sub}' => $data[49],
                'ClassJob{Use}' => intval($data[50]),
                /* data[51] */
                'Damage{Phys}' => intval($data[52]),
                'Damage{Mag}' => intval($data[53]),
                'Delay<ms>' => intval($data[54]),
                /* data[55] */
                'BlockRate' => intval($data[56]),
                'Block' => intval($data[57]),
                'Defense{Phys}' => intval($data[58]),
                'Defense{Mag}' => intval($data[59]),
                'BaseParam' => [
                ['Param' => intval($data[60]),
                'Value' => intval($data[61])],
                ['Param' => intval($data[62]),
                'Value' => intval($data[63])],
                ['Param' => intval($data[64]),
                'Value' => intval($data[65])],
                ['Param' => intval($data[66]),
                'Value' => intval($data[67])],
                ['Param' => intval($data[68]),
                'Value' => intval($data[69])],
                ['Param' => intval($data[70]),
                'Value' => intval($data[71])]],
                'ItemSpecialBonus' => intval($data[72]),
                'ItemSpecialBonus{Param}' => intval($data[73]),
                'BaseParam{Special}' => [
                ['Param' => intval($data[74]),
                'Value' => intval($data[75])],
                ['Param' => intval($data[76]),
                'Value' => intval($data[77])],
                ['Param' => intval($data[78]),
                'Value' => intval($data[79])],
                ['Param' => intval($data[80]),
                'Value' => intval($data[81])],
                ['Param' => intval($data[82]),
                'Value' => intval($data[83])],
                ['Param' => intval($data[84]),
                'Value' => intval($data[85])]],
                'MaterializeType' => intval($data[86]),
                'MateriaSlotCount' => intval($data[87]),
                'IsAdvancedMeldingPermitted' => $this->isTrue($data[88]),
                'IsPvP' => $this->isTrue($data[89]),
                'SubStatCategory' => intval($data[90]),
                'IsGlamourous' => $this->isTrue($data[91])
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

                $sql .= "INSERT INTO `items` (Id, Name, Data) "."VALUES ($id, $name, $data); ";
                $batch -= 1;
                if ($batch <= 0) {
                    DB::run($sql);
                    $batch = $max_batch;
                    $sql = '';
                    print('.');
                }
            }
            DB::run($sql);
            print(".\n");
            fclose($handle);
            $this->itemsLoaded = true;
        }

    }


    private function checkItemTable()
    {
        if ($this->itemsLoaded) {
            return;
        }
        $force = false;
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
            $force = true;
        }
        $this->loadItems($force);

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
                $this->recipiesLoaded = true;
                return;
            }
        }

        $this->loadRecipeLevelTable();

        $sql = "DELETE FROM `recipes` WHERE 1";
        DB::run($sql);

        $handle = fopen($this->baseData.$this->recipeData, "r");
        $skip = 3;
        $max_batch = 500;
        if (empty($handle) === false) {
            $batch = $max_batch;
            $sql = '';
            while (($data = fgetcsv($handle, 1000, ",")) !== false) {
                if ($skip) {
                    $skip--;
                    continue;
                }
                if ($data[0] === '0' || $data[1] === '0') {
                    continue;
                }
                $entry = ['Index' => intval($data[0]),
                'Number' => intval($data[1]),
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
                'Amount' => intval($data[21]),]],
                'RecipeNotebookList' => $data[22],
                'DisplayPriority' => $data[23],
                'IsSecondary' => $this->isTrue($data[24]),
                'MaterialQualityFactor' => $data[25],
                'DifficultyFactor' => $data[26],
                'QualityFactor' => $data[27],
                'DurabilityFactor' => $data[28],
                'RequiredQuality' => $data[29],
                'RequiredCraftsmanship' => $data[30],
                'RequiredControl' => $data[31],
                'QuickSynthCraftsmanship' => $data[32],
                'QuickSynthControl' => $data[33],
                'SecretRecipeBook' => $data[34],
                'Quest' => $data[35],
                'CanQuickSynth' => $this->isTrue($data[36]),
                'CanHq' => $this->isTrue($data[37]),
                'ExpRewarded' => $this->isTrue($data[38]),
                'Status{Required}' => $data[39],
                'Item{Required}' => $data[40],
                'IsSpecializationRequired' => $this->isTrue($data[41]),
                'IsExpert' => $this->isTrue($data[42]),
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

                $sql .= "INSERT INTO `recipes` (Id, Item, CraftType,"."RecipeLevel, Book, Data) "."VALUES ($id, $item, $type, $level, $book, $data); ";

                foreach ($entry['Ingredients'] as $ingred) {
                    if ($ingred['Item'] > 0) {
                        $i = strval($ingred['Item']);
                        $a = strval($ingred['Amount']);
                        $sql .= "INSERT INTO `ingredients` (Id, Recipe, Item, Amount, Level) VALUES ($id, $item, $i, $a, $level); ";
                    }
                }

                $batch -= 1;
                if ($batch <= 0) {
                    DB::run($sql);
                    $batch = $max_batch;
                    $sql = '';
                    print(".");
                }
            }
            DB::run($sql);
            print(".\n");
            fclose($handle);
            $this->recipiesLoaded = true;
        }

    }


    private function checkRecipeTable()
    {
        if ($this->recipiesLoaded) {
            return;
        }
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
                ALTER TABLE `recipes` ADD INDEX(`Item`);
                ALTER TABLE `recipes` ADD INDEX(`Book`);
SQL;
            DB::run($sql);
            $force = true;
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
                ALTER TABLE `ingredients` ADD INDEX(`Item`);
                ALTER TABLE `ingredients` ADD INDEX(`Recipe`);
SQL;
            DB::run($sql);
            $force = true;
        }
        $this->loadRecipes($force);

    }


    private function checkNeededTable()
    {
        $sql = "SHOW TABLES LIKE 'needed';";
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value === false) {
            $sql = <<<SQL
                CREATE TABLE needed (
                    ItemId INT UNSIGNED NOT NULL PRIMARY KEY,
                    Data TEXT NOT NULL
                );
SQL;
            DB::run($sql);
        }

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
                if (!$this->isTrue($data[6])) {
                    continue;
                }
                $entry = [
                    'Index' => intval($data[0]),
                    'InternalName' => $data[1],
                    'Name' => $data[2],
                    'Region' => intval($data[3]),
                    'UserType' => intval($data[4]),
                    'DataCenter' => intval($data[5]),
                    'Public' => $this->isTrue($data[6]),
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
                $this->gilShop[intval($data[1])] = $entry;
            }
            fclose($handle);
        }

    }


    public function findItemInShop($itemID)
    {
        $this->loadGilShop();
        $result = [];
        if (array_key_exists($itemID, $this->gilShop)) {
            $result[] = $this->gilShop[$itemID];
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
        if (is_array($tgt)) {
            $tgt = $tgt[0];
        }
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
            if (is_array($part)) {
                $part = $part[0];
            }
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


    public function getItem($param = null)
    {
        $this->checkItemTable();
        if (is_numeric($param)) {
            $sql = "SELECT `Data` FROM `items` WHERE `ID` = $param;";
        } elseif ($param) {
            $p = MyPDO::quote($param);
            $sql = "SELECT `Data` FROM `items` WHERE lower(Name) = $p;";
        } else {
            $sql = "SELECT `Data` FROM `items`;";
        }
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value !== false) {
            return json_decode($value['Data']);
        }
        return null;

    }


    public function getItems($param = null)
    {
        $this->checkItemTable();
        if (is_numeric($param)) {
            $sql = "SELECT `ID` FROM `items` WHERE `ID` = $param;";
        } elseif ($param) {
            $param = strtolower($param);
            $sql = "SELECT `ID` FROM `items` WHERE lower(Name) LIKE '%$param%';";
        } else {
            $sql = "SELECT * FROM `items`;";
        }
        $result = DB::run($sql);
        $value = $result->fetch();
        $output = null;
        while ($value !== false) {
            if ($output === null) {
                $output = [];
            }
            if ($param) {
                $output[] = $value['ID'];
            } else {
                $output[] = json_decode($value['Data']);
            }
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


    public function getRecipeSet($crafter = null, $levelLow = 0, $levelHigh = 0, $roweena=false, $includeExpert=false)
    {
        $this->loadRecipeLevelTable();
        $this->checkRecipeTable();
        $excludeCraft = false;
        if (!is_null($crafter) && !is_numeric($crafter)) {
            $crafter = array_search($crafter, $this->craftType);
            if ($crafter === false) {
                return null;
            }
        } elseif (is_null($crafter)) {
            /* no cooking in big lists */
            $crafter = 7;
            $excludeCraft = true;
        }
        $sql = "SELECT `Item`, `Data` FROM `recipes` WHERE ";
        $and = false;
        if (!is_null($crafter)) {
            if ($excludeCraft) {
                $sql .= " NOT `CraftType` = '$crafter'";
            } else {
                $sql .= " `CraftType` = '$crafter'";
            }
            $and = true;
        }
        if ($levelLow > 0) {
            if ($and) {
                $sql .= " AND ";
            }
            $sql .= " `RecipeLevel` >= $levelLow";
            $and = true;
        }
        if ($levelHigh > 0) {
            if ($and) {
                $sql .= " AND ";
            }
            $sql .= " `RecipeLevel` <= $levelHigh";
            $and = true;
        }
        if ($and) {
            $sql .= " AND ";
        }
        $sql .= " `Book` IS NULL";
        $result = DB::run($sql);
        $value = $result->fetch();
        $output = null;
        while ($value !== false) {
            $item = $this->getItem($value['Item']);
            if ($roweena) {
                if (is_null($item) || !strstr($item->Name, 'Rarefied')) {
                    $value = $result->fetch();
                    continue;
                }
            } elseif (is_null($item) || $item->IsUntradable || $item->IsIndisposable) {
                $value = $result->fetch();
                continue;
            }
            if (!$includeExpert) {
                $data = json_decode($value['Data']);
                if ($data->IsExpert) {
                    $value = $result->fetch();
                    continue;
                }
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


    public function getNeeded($item)
    {
        $this->checkNeededTable();
        $sql = "SELECT * FROM `needed` WHERE ItemId = $item;";
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value !== false) {
            return(json_decode($value['Data'], true));
        }
        return null;

    }


    public function setNeeded($item, $ingred)
    {
        $this->checkNeededTable();
        $data = MyPDO::quote(json_encode($ingred));
        $sql = "INSERT INTO `needed` (ItemId, Data) VALUES ($item, $data); ";
        $result = DB::run($sql);

    }


    public function resetAndReload()
    {
        $sql = "DROP TABLE IF EXISTS `recipes`;";
        DB::run($sql);
        $sql = "DROP TABLE IF EXISTS `ingredients`;";
        DB::run($sql);
        $sql = "DROP TABLE IF EXISTS `items`;";
        DB::run($sql);
        $sql = "DROP TABLE IF EXISTS `needed`;";
        DB::run($sql);

        $this->itemsLoaded = false;
        $this->checkItemTable();
        $this->recipiesLoaded = false;
        $this->checkRecipeTable();

    }


    public function resetNeeded()
    {
        $sql = "DROP TABLE IF EXISTS `needed`;";
        DB::run($sql);

    }


    /* End */
}
