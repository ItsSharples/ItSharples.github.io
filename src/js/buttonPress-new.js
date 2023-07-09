var hamburger = document.querySelector("label[id=hamburger]");
var showMenu = document.querySelector("input[id=showMenu]");

showMenu.addEventListener('change', function() {
    this.checked ? hamburger.setAttribute('checked', "") : hamburger.removeAttribute('checked');
});