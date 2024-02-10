const form = document.getElementById('login');

const HOST = window.location.host

async function hashString(message) {
    const encoder = new TextEncoder();
    const data = encoder.encode(message);
    const hashBuffer = await crypto.subtle.digest("SHA-256", data);
    const hashArr = Array.from(new Uint8Array(hashBuffer));
    const hashHex = hashArr
        .map((b) => b.toString(16).padStart(2, '0'))
        .join('');
    return hashHex;
}

async function handleSubmit() {

    const usernameElement = form.elements['username'];
    const passwordElement = form.elements['password'];

    let username = usernameElement.value;
    let passwordHash = await hashString(passwordElement.value);

    let body = { 'username': username, 'passwordHash': passwordHash};

    try {
        const response = await fetch("http://" + HOST + "/trylogin", {
            method: "POST",
            body: JSON.stringify(body),
        });

        console.log(await response);
        window.location.href = response.url
    } catch (e) {
        console.log(e);
    }

}

form.addEventListener('submit', (event) => {
    
    event.preventDefault();
    handleSubmit();

});