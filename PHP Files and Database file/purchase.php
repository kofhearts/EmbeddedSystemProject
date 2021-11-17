
<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
$itemPrice1 = 20;
$itemPrice2=25;
$itemPrice3=18;
$chooseItem = $_GET['chooseItem'];
$quantity = $_GET['quantity'];

    if(isset($_GET['chooseItem']) && isset($_GET['card']) && isset($_GET['quantity'])){
            
        $get_card = $_GET['card']."-balance.txt";
        $card_balance = file_get_contents($get_card);
        
        if($chooseItem == 1){
            $total = $quantity * $itemPrice1;
        }
        
        if($chooseItem == 2){
            $total = $quantity * $itemPrice2;
        }
        
        if($chooseItem == 3){
            $total = $quantity * $itemPrice3;
        }
        
        if($card_balance > $total){
            file_put_contents($get_card, $card_balance-$total); 
            echo "1";

        }else if($card_balance < $total){
            echo "0";
        }
       
        
    }


?>