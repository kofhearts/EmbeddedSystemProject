<?php
    ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

    if(isset($_GET['card']) && isset($_GET['pin'])){
        
        $card_file = $_GET['card']."-pin.txt";
        $card_pin = file_get_contents($card_file);
        if($card_pin == $_GET['pin']){
            echo "1";
        }else{
            echo "0";
        }
    }


?>