alert("Welcome to the Password Game!");
alert("Here's how to play the game: \n 1st. Follow what the rules say. \n 2nd. Type in the password in the input. \n Don't worry! This game, It's easy!");
var play = confirm("Do you want to play?");
if (play == true) {
    alert("Great! Let's start typing!");
    document.getElementById("game").setAttribute("src", "password.js");
}
else {
    window.history.back;
}