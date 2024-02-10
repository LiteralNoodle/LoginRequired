<?php
session_start();

// Filler values for correct username and password
//$validUsername = 'your_username';
//$validPassword = 'your_password';
$hashalgo = "sha256";
$myfile = fopen("users", "r");
$userdatabase = fread($myfile,filesize("users"));
$inputlist = explode(" ",$userdatabase);
//$validusernames = array();
//for ($i=0; $i < count($inputlist); $i+=2){
//   $validusernames[$i/2] = $inputlist[$i];
//}

// Check if the form is submitted
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $enteredUsername = $_POST["username"];
    $enteredPassword = $_POST["password"];
    $hashedEnteredPassword = hash($hashalgo, $enteredPassword);
    // Validate the entered credentials
    for ($i = 0; $i < count($inputlist); $i++){
            if ($enteredUsername == $inputlist[$i] && $hashedEnteredPassword == $inputlist[$i+=1]) {
        // Authentication successful
                $_SESSION["username"] = $enteredUsername;
                header("Location: logged_in.php");
                exit();
    }       
            else {
        // Authentication failed
                $error_message = "Invalid username or password";
    }
}
}

?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login Page</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            text-align: center;
            padding: 50px;
        }

        #login-form {
            max-width: 300px;
            margin: 0 auto;
            background-color: #fff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }

        input {
            width: 100%;
            padding: 8px;
            margin-bottom: 10px;
            box-sizing: border-box;
        }

        button {
            background-color: #4caf50;
            color: white;
            padding: 10px 15px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <div id="login-form">
        <h2>Login</h2>
        <form action="" method="post">
            <label for="username">Username:</label>
            <input type="text" id="username" name="username" required>

            <label for="password">Password:</label>
            <input type="password" id="password" name="password" required>

            <button type="submit">Login</button>
        </form>

        <?php
        if (isset($error_message)) {
            echo "<p style='color: red;'>$error_message</p>";
        }
        ?>
    </div>
</body>
</html>