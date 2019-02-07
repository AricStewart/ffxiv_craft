<?php
/**
 * Access to datafiles from https//github.com/viion/ffxiv-datamining
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
    require_module 'curl';
    require_module 'array';
    require_module 'core';
    require_module 'file';
    require_module 'phpinfo';
.*/

ini_set('memory_limit', '5000M');

function _worldSort($a, $b) 
{
    return strnatcmp($a['Name'], $b['Name']);
}

class FfxivDataSet
{
    public $recipe = array();
    public $item = array();
    public $gather = array();
    public $master = array();
    public $world = array();

    public $recipData = "/data/Recipe.csv";
    public $itemData = "/data/Item.csv";
    public $gatherData = "/data/GatheringItem.csv";
    public $masterData = "/data/SecretRecipeBook.csv";
    public $worldData = "/data/World.csv";

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

    function getData($file, $url) 
    {
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        $content = curl_exec($ch);
        curl_close($ch);
        file_put_contents($file, $content);
        chmod($file, 0666);
    }

    function updateData() 
    {
        $itemURL = "https://raw.githubusercontent.com/viion/ffxiv-datamining/".
                    "master/csv/Item.csv";
        $recipURL = "https://raw.githubusercontent.com/viion/ffxiv-datamining/".
                    "master/csv/Recipe.csv";
        $gatherURL = "https://raw.githubusercontent.com/viion/ffxiv-datamining/".
                    "master/csv/GatheringItem.csv";
        $masterURL = "https://raw.githubusercontent.com/viion/ffxiv-datamining/".
                    "master/csv/SecretRecipeBook.csv";
        $this->getData($this->baseData.$this->recipData, $recipURL);
        $this->getData($this->baseData.$this->itemData, $itemURL);
        $this->getData($this->baseData.$this->gatherData, $gatherURL);
        $this->getData($this->baseData.$this->masterData, $masterURL);
    }

    function __construct($baseData='.') 
    {
        $this->baseData = $baseData;
        if (!file_exists($this->baseData.$this->recipData)) {
            print "Updating Data...\n";
            $this->updateData();
        }
    }

    function loadRecipies()
    {
        if (count($this->recipe) > 0) {
            return;
        }
        $handle = fopen($this->baseData.$this->recipData, "r");
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
                $this->recipe[$data[4]] = $entry;
            }
            fclose($handle);
        }
    }

    function loadItems() 
    {
        if (count($this->item) > 0) {
            return;
        }
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
if (count($data) < 90) {
var_dump($data);
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
                    'Price{Mid}' => intval($data[25]),
                    'Price{Low}' => intval($data[26]),
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

                $this->item[$data[0]] = $entry;
            }
            fclose($handle);
        }
    }

    function loadGathering() 
    {
        if (count($this->gather) > 0) {
            return;
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
                $this->master[] = $entry;
            }
            fclose($handle);
        }
    }

    function loadWorld() 
    {
        if (count($this->world) > 0) {
            return;
        }
        if (!file_exists($this->baseData.$this->worldData)) {
            $worldURL = "https://raw.githubusercontent.com/viion/".
                        "ffxiv-datamining/master/csv/World.csv";
            $this->getData($this->baseData.$this->worldData, $worldURL);
        }
        $handle = fopen($this->baseData.$this->worldData, "r");
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
                $this->world[] = $entry;
            }
            usort($this->world, "_worldSort");
            fclose($handle);
        }
    }

    function getFullRecipe($id, $count=1, &$output=null) 
    {
        $this->loadRecipies();
        $this->loadItems();
        if ($output == null) {
            $output = array();
        }
        $tgt = $this->recipe[$id];
        foreach ($tgt['Ingredients'] as $i) {
            $bits = [];
            if (array_key_exists($i['Item'], $this->recipe) &&
                $this->recipe[$i['Item']] !== null) {
                $result = $this->recipe[$i['Item']]['Result']['Amount'];
                $this->getFullRecipe(
                    $i['Item'],
                    ceil(($i['Amount']* $count)/$result), $bits
                );
            }
            $output[] = [
                         'id' => $i['Item'],
                         'name' => $this->item[$i['Item']]['Name'],
                         'count' => $i['Amount'] * $count,
                         'marketCost' => 0,
                         'marketHQ' => 0,
                         'craftCost' => 0,
                         'bits' => $bits
                        ];
        }
        return $output;
    }


    function lookupMasterBook($name) 
    {
        $this->loadMasterBooks();
        if (is_int($name)) {
            return $name;
        }
        foreach ($this->master as $r) {
            if (strcasecmp($r['Name'], $name) == 0) {
                return $r['Index'];
            }
        }
        return null;
    }

    function getMasterCraft($book=null) 
    {
        $this->loadRecipies();
        $output = [];
        if ($book !== null) {
            $theBook = $this->lookupMasterBook($book);
            if ($theBook === null) {
                print "Failed to find volume ".$book."\n";
                return null;
            }
            $book = $theBook;
        }
        foreach ($this->recipe as $key => $recp) {
            if ($recp['SecretRecipeBook']) {
                if ($book !== null && $book == $recp['SecretRecipeBook']) {
                    $output[] = $key;
                } elseif ($book === null) {
                    $output[] = $key;
                }
            }
        }
        return $output;
    }

    function getItemByName($name)
    {
        $this->loadItems();
        foreach ($this->item as $key => $val) {
            if (strcasecmp($val['Name'], $name) == 0) {
                return $key;
            }
        }
        return null;
    }
}
