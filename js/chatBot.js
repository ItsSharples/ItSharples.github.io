


const hash = window.location.hash.substring(1);
var params = {}
hash.split('&').map(hk => {
    let temp = hk.split('=');
    params[temp[0]] = temp[1]
});
console.log(params); //Here are the params to use

document.getElementById("subbody").innerHTML = params.access_token.toString()