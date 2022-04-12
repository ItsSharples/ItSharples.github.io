
if(window.location.hash)
{
    runChat(window.location.hash.substring(1));
}
else
{
    runConfig();
}

function runConfig() {
    document.getElementById("subbody").innerHTML = "CONFIG MODE"
}

function runChat(hash) {
    var params = {}
    hash.split('&').map(hk => {
        let temp = hk.split('=');
        params[temp[0]] = temp[1]
    });
    console.log(params); //Here are the params to use

    document.getElementById("subbody").innerHTML = "Chatbot mode" + params.access_token.toString()
}