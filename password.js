test();
function test() {
    var password = document.getElementById("password").value;
    if (password == "a") {
        document.getElementById("rule").innerHTML = "Rule 2. Type the first number.";
    }
    if (password == "a1") {
        document.getElementById("rule").innerHTML = "Rule 3. Type in a planet that we live on right now.";
    }
    if (password == "a1earth") {
        document.getElementById("rule").innerHTML = "Rule 4. Type a star that we see when it's daytime.";
    }
    if (password == "a1earthsun") {
        document.getElementById("rule").innerHTML = "Rule 5. 8 x 8?";
    }
    if (password == "a1earthsun64") {
        document.getElementById("rule").innerHTML = "Rule 6. Type in \"daddy\" backwards.";
    }
    if (password == "a1earthsun64yddad") {
        document.getElementById("rule").innerHTML = "Rule 7. Type a Uppercase letter but the word \"zebra\".";
    }
    if (password == "a1earthsun64yddadZebra") {
        document.getElementById("rule").innerHTML = "Rule 8. What is the richest country ever? Hint: It is not what stands for, It starts with \"United\".";
    }
    if (password == "a1earthsun64yddadZebraunitedstates") {
        document.getElementById("rule").innerHTML = "Rule 9. What is the strictest, baddest country ever? It is one of the Korean Countries.";
    }
    if (password == "a1earthsun64yddadZebraunitedstatesnorthkorea") {
        document.getElementById("rule").innerHTML = "Rule 10. Put a space at the end of your password.";
    }
    if (password == "a1earthsun64yddadZebraunitedstatesnorthkorea ") {
        document.getElementById("rule").innerHTML = "Rule 11. Type a card game that you play by yourself.";
    }
    if (password == "a1earthsun64yddadZebraunitedstatesnorthkorea solitaire") {
        document.getElementById("rule").innerHTML = "Rule 12. Type in the year when World War 2 had ended.";
    }
    if (password == "a1earthsun64yddadZebraunitedstatesnorthkorea solitaire1945") {
        document.getElementById("rule").innerHTML = "Rule 13. Type in the city that is \"The heart of the fillipino\".";
    }
    if (password == "a1earthsun64yddadZebraunitedstatesnorthkorea solitaire1945manila") {
        document.getElementById("rule").innerHTML = "Rule 14. What is the flavor that is brown?";
    }
/* all random question you can think of here. */
/* When win */
    if (password == "a1earthsun64yddadZebraunitedstatesnorthkorea solitaire1945manilachocolate") {
        alert("Congratulations, You won the password game!");
var playagain = confirm("Would you like to play again?");
if (playagain == true) {
    location.reload();
}
else {
    alert("Canceled.");
    window.history.back();
}
    }
    document.getElementById("number").innerHTML = password.length;
    const update = setTimeout(test, 10); 
}
