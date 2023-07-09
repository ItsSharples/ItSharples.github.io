


const tableDOM = document.querySelector("table")
const exampleModeCheck = document.getElementById("example-mode");

const debugMode = true;
let tableState = {
    "Headers" : [],
    "NumAssignments" : 0,
    "Contents" : [],
}

let history = [tableState]
var beforeRow = -1;
var creationDialog;
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/random
function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max-min) + min);
  }
function getRandomAssignmentGrade() {
    const num = getRandomInt(20, 55)
    if(num > 50){return 0}
    return num * 2;
}
function getRandomName(){
    names = [
        "Cora Hurley",
        "Jerome Goodwin",
        "Alijah Roth",
        "Arturo House",
        "Cecilia Gomez",
        "Eleanor Berry",
        "Ashtyn Warren",
        "Josh Hicks",
        "Axel Orr",
        "Coby Arias",
        "Bronson Steele",
        "Nikhil Dalton",
        "Marlie Davila",
        "Darion Mullen",
        "Adriana Crosby",
        "Hassan Gaines",
        "Casey Williamson",
        "Javon Gonzales",
        "Keely Yates",
        "Jocelyn Pena",
        "Simeon Cunningham",
        "Ava Oneal",
        "Kenyon Deleon",
        "Libby Chambers",
        "Lila Howell",
        "Anaya Lewis",
        "Noelle Mason",
        "Mohammed Williamson",
        "Allie Walters",
        "Samuel Lambert",
        "Holly Nichols",
        "Brooklynn Ellison",
        "Jillian Shea",
        "Shirley Hanson",
        "Turner Knox",
        "Conor Pace",
        "George Snyder",
        "Clay Padilla",
        "Jordin Mooney",
        "Colten Hodges",
        "Kylan Reed",
        "Leland Yang",
        "Reece Harrington",
        "Reyna Morgan",
        "Ciara Kline",
        "Slade Brooks",
        "Blaine Beltran",
        "Peter Joyce",
        "Orlando Stevenson",
        "Hezekiah Merritt"
    ]
    let firstNames = []
    let lastNames = []
    for(name of names){
        splitNames = name.split(" ")
        firstNames.push(splitNames[0])
        lastNames.push(splitNames[1])
    }
    CharName = [firstNames[getRandomInt(0, firstNames.length)], lastNames[getRandomInt(0, firstNames.length)]].join(" ")
    return CharName;
}
const getRandomStudentID = () => getRandomInt(2100000, 2199999);
const validIndex = (index) => Math.min(Math.max(index, 0), tableState.Contents.length) == index;

function saveToHistory(){
    if(JSON.stringify(tableState) !== JSON.stringify(history.slice(-1))){
        history.push(JSON.parse(JSON.stringify(tableState)))
    }
}
function reloadToLastHistory(){
    console.log(history)
    let currState = history.pop()
    setTableTo(history.pop())
}

function htmlString(string){
    var out = string.toLowerCase()
    out = out.replaceAll(" ", "-")
    return out;
}
function setTableTo(data){
    tableState["Headers"] = data["Headers"]
    tableState["NumAssignments"] = data["NumAssignments"]
    tableState["Contents"] = data["Contents"]

    saveToHistory()
    regenerateTableDOM()
}
function resetTable(){
    console.log("Reset Table")
    tableState["Headers"] = ["StudentName", "StudentID",]
    tableState["NumAssignments"] = 5
    tableState["Contents"] = []

    for(newChar of [...Array(10)]){
        AssignmentGrades = [...Array(tableState.NumAssignments)].map((value, index, array)=>getRandomAssignmentGrade());
        tableState.Contents.push([getRandomName(), getRandomStudentID(), AssignmentGrades])
    }

    saveToHistory()
    regenerateTableDOM()
}

function saveTable() {
    console.log("Save Table")
    localStorage.setItem("data", JSON.stringify(tableState));
}

function loadTable() {
    console.log("Load Table")
    var data = JSON.parse(localStorage.getItem("data"));
    setTableTo(data)
}
function createStudent(beforeRow = -1) {
    if(exampleModeCheck.checked){
        addRow(null, null, beforeRow)
    }
    else{
        creationDialog.show()
    }
}
function addRow(name, id, before = -1) {
    console.log(`Add Row: ${name}`)
    let AssignmentGrades = []
    if(exampleModeCheck.checked){
        name = getRandomName();
        id = getRandomStudentID();
        for(val of [...Array(tableState.NumAssignments)]){
            AssignmentGrades.push(getRandomAssignmentGrade())
        }
    }

    if (!name || !id) {
        console.log("Invalid Inputs")
        return;
    }

    var newStudent = [name, id, AssignmentGrades];
    /// Splice can work on negative indices, but we're using -1 to mean unassigned
    if(before == -1){
        tableState.Contents.push(newStudent)
    }
    else{
        tableState.Contents.splice(before, 0, newStudent)
    }
    saveToHistory()
    regenerateTableDOM()
}

function removeRow(index){
    console.log(`Remove Row: ${index}`)
    
    if(!validIndex(index)){
        console.log(`Index out of range: ${index}`)
        return;
    }
    tableState.Contents.splice(index, 1)
    saveToHistory()
    regenerateTableDOM()
}

function isColumnEmpty(index){
    return tableState.Contents.every((row) => row[2][index] == 0)
}
function addColumn(name = "", position = -1, defaultValue = 0) {
    if(name == ""){
        name = `Assignment ${position == -1 ? tableState.NumAssignments + 1 : position}`
    }
    console.log(`Add Column: ${name}`)
    tableState.NumAssignments += 1;
    for(character of tableState.Contents){

    if(position == -1){
        position = character[2].length
        character[2].push(defaultValue)
    }
    else{
        character[2].splice(position, 0, defaultValue)
    }

    if(exampleModeCheck.checked){
            character[2][position] = getRandomAssignmentGrade()
        }
    }
    saveToHistory()
    regenerateTableDOM();
}

function removeColumn(forceAction, index){
    console.log(`Remove Assignment: ${index}`)
    const isEmpty = isColumnEmpty(index)

    if(forceAction || isEmpty){ // Remove the Column
        tableState.Contents = tableState.Contents.map((row) => {
            var assignmentValues = row.pop()
            assignmentValues.splice(index, 1)
            var newRow = row.concat([])
            newRow[2] = [...assignmentValues]
            return newRow
            
        })
        tableState.NumAssignments = tableState.NumAssignments - 1;
    }
    else { // Empty the Column
        for(row of tableState.Contents){
            row[2][index] = 0
        }
    }
    saveToHistory()
    regenerateTableDOM()
}

function clearTableDOM(){
    console.log("Clear Table DOM")
    while (tableDOM.firstChild) {
        // The list is LIVE so it will re-index each call
        tableDOM.removeChild(tableDOM.firstChild);
      }
}
// Doesn't work :angy:
function moveFocusEnd(cell) {
    console.log(cell)
    cell.focus()
    var tmp = cell.firstChild.textContent;
    cell.firstChild.textContent = '';
    cell.firstChild.textContent = tmp;
}
function onEditGrade(event){
    console.log("Edit Grade");
    var newVal = Number.parseInt(event.target.firstChild.data)
    if(isNaN(newVal)){
        event.preventDefault()
        console.log(`Invalid Input ${newVal}`)
        return
    }
    newVal = Math.max(newVal, 0);
    newVal = Math.min(newVal, 100);
    newVal = newVal == 0 ? "" : newVal;
    {
        // This messes up where the selection is, but meh
        event.target.firstChild.data = newVal

        const changedRow = event.target.parentElement.rowIndex - 1
        const changedCol = event.target.cellIndex - 2;
        tableState.Contents[changedRow][2][changedCol] = newVal;
        console.log(tableState);

        saveToHistory()
        regenerateTableDOM([changedRow, changedCol])
    }
}

function getGradeText(percentGrade, type = "percent"){
    const mapTo = {
            60 : ["F", "0.0"],
            62 : ["D-", "0.7"],
            66 : ["D", "1.0"],
            69 : ["D+", "1.3"],
            72 : ["C-", "1.7"],
            66 : ["C", "2.0"],
            69 : ["C+", "2.3"],
            82 : ["B-", "2.7"],
            86 : ["B", "3.0"],
            89 : ["B+", "3.3"],
            92 : ["A-", "3.7"],
            100 : ["A", "4.0"]
    }
    selectedGrade = []
    for(key of Object.keys(mapTo)){
        if (Number.parseInt(key) > percentGrade)
        {
            selectedGrade = mapTo[key]
            break;
        }
    }
    switch (type){
        case "percent":
            return percentGrade;
        case "letter":
            return selectedGrade[0];
        case "number":
            return selectedGrade[1];
    } 
    
}
function getGrade(values, type = "percent"){
    var percent = values.reduce((acc, curr) => acc + curr, 0) / tableState.NumAssignments;
    percent = Math.round(percent);
    return {"Pass" : percent > 60, "Text": getGradeText(percent, type)};
}
function updateGradeText(forCell, withValues, withType){
    const grade = getGrade(withValues, withType);
    forCell.textContent = grade.Text; 
    if(grade.Pass){forCell.classList.remove("fail-grade");}
    else{forCell.classList.add("fail-grade");}
}

function makeButton(name, id = "", clickEvent = null){
    const newButton = document.createElement("button");
    newButton.id = id;
    newButton.textContent = name

    newButton.addEventListener("click", clickEvent)
    return newButton
}

// https://developer.mozilla.org/en-US/docs/Web/API/Document_Object_Model/Traversing_an_HTML_table_with_JavaScript_and_DOM_Interfaces
// https://javascript.plainenglish.io/how-to-listen-for-changes-to-html-elements-with-the-contenteditable-attribute-with-javascript-b715eeb4431e


/* https://www.matuzo.at/blog/highlighting-columns/ */
/* https://stackoverflow.com/a/42658933 */

function selectRow(rowElement){
    const didntContain = !rowElement.classList.contains("highlight-click-row");
    for(element of document.getElementsByClassName("highlight-click-row")){
        element.classList.remove("highlight-click-row");
    }

    if(didntContain){
        rowElement.classList.add("highlight-click-row")
    }
}
function regenerateTableDOM(focusOn = []){
    //if(saveChange){saveToHistory()}
    console.log("Regenerate Table DOM")
    const tblBody = document.createElement("tbody");
    const tblHead = document.createElement("thead");
    const colGroup = document.createElement("colgroup");

    const gradeSelect = document.createElement("select");

    let shouldFocus = focusOn != [];
    let focusCell = null;

    // generate the headers
    const headerRow = document.createElement("tr");
    headerRow.id = "header-row";
    for (const rowData of tableState.Headers) {
        const cell = document.createElement("th");
        const cellText = document.createTextNode(rowData);
        cell.appendChild(cellText);
        cell.scope = "col";
        headerRow.appendChild(cell);

        const col = document.createElement("col");
        col.classList.add(`${rowData}-col`);
        colGroup.appendChild(col);
    }
    // Add the Assignment Headers
    Array(tableState.NumAssignments).fill().map((_, assignmentNum) => {
        const cell = document.createElement("th");
        const cellText = document.createTextNode(`Assignment ${assignmentNum + 1}`);
        cell.appendChild(cellText);
        cell.scope = "col";
        headerRow.appendChild(cell);

        const col = document.createElement("col");
        col.classList.add("assignment-col");
        colGroup.appendChild(col);
    }) 

    // The Final Grade Column is going to be funky, so just add it specifically
    {
        const gradeCell = document.createElement("th");
        gradeCell.scope = "col";
        const cellText = document.createTextNode("Final Grade");

        const gradeLabel = document.createElement("label");
        //gradeSelect = document.createElement("select");
        
        const gradePercent = document.createElement("option");
        gradePercent.value = "percent";
        gradePercent.textContent = "Average [%]";
        const gradeLetter = document.createElement("option");
        gradeLetter.value = "letter";
        gradeLetter.textContent = "Average [Letter]";
        const gradeNumber = document.createElement("option");
        gradeNumber.value = "number";
        gradeNumber.textContent = "Average [4.0]";

        gradePercent.setAttribute("selected", true);

        gradeSelect.classList.add("grade-selector")
        gradeSelect.add(gradePercent);
        gradeSelect.add(gradeLetter);
        gradeSelect.add(gradeNumber);

        //gradeCell.appendChild(cellText);
        gradeCell.appendChild(gradeSelect);
        headerRow.appendChild(gradeCell);

        const col = document.createElement("col");
        col.classList.add("final-grade-col");
        colGroup.appendChild(col);
    }

    // Add the headers to the body
    tblHead.appendChild(headerRow);

    var currIndex = 0;
    /// Now we add the Contents to the Table
    for (const rowData of tableState.Contents) {
        const row = document.createElement("tr");
        row.id = `student-${htmlString(rowData[0])}-${rowData[1]}`
        row.name = `student-${htmlString(rowData[0])}-${rowData[1]}`
        row.value = row.id;
        const focusOnRow = shouldFocus & (focusOn[0] == tblBody.childElementCount);
        
        let gradeValues = []
        for (const columnData of rowData) {
            // If it's a list, assume it be the assignment values
            if(Object.prototype.toString.call(columnData) == "[object Array]"){
                gradeValues = columnData
                Array(tableState.NumAssignments).fill().map((_, assignmentNum) => {
                    const cell = document.createElement("td");
                    const scoreGot = gradeValues[assignmentNum];
                    // If there's a score, write that, else write nothing
                    cell.textContent = !!scoreGot ? scoreGot : "";
                    cell.classList.add("grade")

                    row.appendChild(cell);

                    // https://stackoverflow.com/a/20906991
                    // Add Content Editable Attribute
                    cell.contentEditable = "true";
                    cell.inputMode = "numeric"

                    cell.addEventListener("input", onEditGrade);

                    const focusOnCell = focusOnRow & (focusOn[1] == assignmentNum)
                    if (focusOnCell){focusCell = cell; shouldFocus = false;}
                }) 
            }
            else {
                const cell = document.createElement("td");
                cell.textContent = columnData;
                row.appendChild(cell);

                if(columnData == rowData[0]){
                    cell.addEventListener("click", (event) => {
                        selectRow(event.target.parentElement);
                    })
                }
            }
        }

        // Work out the final Grade
        const gradeCell = document.createElement("td");
        updateGradeText(gradeCell, gradeValues, gradeSelect.value)
        gradeSelect.addEventListener("change", (event) => {
            updateGradeText(gradeCell, gradeValues, gradeSelect.value)
        });
        gradeCell.classList.add("grade")
        row.appendChild(gradeCell);

        // add the row to the end of the table body
        tblBody.appendChild(row);
    }

    {// Add footer to add the add buttons
        const footer = document.createElement("tfoot");
        const dialog = document.createElement("dialog");
        { // Create Student Dialog
            const title = document.createElement("p");
            const form = document.createElement("form");
            form.method = "dialog";

            const studentNameLabel = document.createElement("label");
            studentNameLabel.htmlFor = "studentName"
            studentNameLabel.textContent = "Student Name";
            const studentIDLabel = document.createElement("label");
            studentIDLabel.htmlFor = "studentID"
            studentIDLabel.textContent = "Student ID";
            const studentName = document.createElement("input");
            studentName.name = "studentName"
            const studentID = document.createElement("input");
            studentID.name = "studentID"
            const submit = document.createElement("button");
            submit.textContent = "OK";
            submit.type = "submit"

            form.title = "New Student"
            form.appendChild(studentNameLabel);
            form.appendChild(studentName);
            form.appendChild(studentIDLabel);
            form.appendChild(studentID);
            form.appendChild(submit);
            form.submit = submit;
            form.onsubmit = (submit) => {
                console.log(submit);
                let name = "";
                let id = "";
                for(child of submit.target){
                    if (child.name == "studentID") {id = child.value}
                    if (child.name == "studentName") {name = child.value}
                }
                addRow(name, id, beforeRow)
                dialog.close();
                
            }

            dialog.appendChild(title);
            dialog.appendChild(form);
        }

        const studentCell = document.createElement("td");
        const addStudentButton = makeButton("Add Student", "new-student-btn", (event) => {
            createStudent()
        });
        creationDialog = dialog;

        addStudentButton.appendChild(dialog);
        studentCell.appendChild(addStudentButton);
        footer.appendChild(studentCell);

        const assignmentCell = document.createElement("td");
        const addAssignmentButton = makeButton("Add Assignment", "new-assignment-btn", (event)=>{addColumn();})
        assignmentCell.appendChild(addAssignmentButton);
        footer.appendChild(assignmentCell);

        const saveCell = document.createElement("td");
        const addSaveButton = makeButton("Save Table", "save-table-btn", (event)=>{saveTable();})
        saveCell.appendChild(addSaveButton);
        footer.appendChild(saveCell);

        const loadCell = document.createElement("td");
        const addLoadButton = makeButton("Load Table", "load-table-btn", (event)=>{loadTable();})
        loadCell.appendChild(addLoadButton);
        footer.appendChild(loadCell);

        const resetCell = document.createElement("td");
        const addResetButton = makeButton("Reset Table", "reset-table-btn", (event)=>{resetTable();})
        resetCell.appendChild(addResetButton);
        footer.appendChild(resetCell);

        tblBody.appendChild(footer);
    }


    // Actually write this to the DOM
    clearTableDOM();
    tableDOM.appendChild(colGroup);
    tableDOM.appendChild(tblHead);
    tableDOM.appendChild(tblBody);
    if(!!focusCell){
        
        moveFocusEnd(focusCell);
    }

    // Add Hover/Click Effect for Columns
    {
        const cols = tableDOM.getElementsByTagName('col');
        const tds = document.getElementsByTagName('td');
        const ths = document.getElementsByTagName('th');
        const columnEnter = (i) => {if(!!cols[i]){cols[i].classList.add('highlight-hover');}}
        const columnLeave = (i) => {if(!!cols[i]){cols[i].classList.remove('highlight-hover');}}

        for (const cell of [...tds].concat([...ths])) {
            const index = cell.cellIndex;
            cell.addEventListener('mouseenter', columnEnter.bind(this, index));
            cell.addEventListener('mouseleave', columnLeave.bind(this, index));
        }
        for (const cell of [...ths]) {
            const index = cell.cellIndex;
            if(!!cols[index]){
            if(!cols[index].classList.contains("final-grade-col")) {
            cell.addEventListener('click', (event) => {
                
                    const col = cols[index];
                    const didntContain = !col.classList.contains("highlight-click-col");
                    for(element of document.getElementsByClassName("highlight-click-col")){
                        element.classList.remove("highlight-click-col");
                    }

                    if(didntContain){
                        col.classList.add("highlight-click-col")
                    }
                }
            );
        }
    }
    }
}
}

function activateContextMenu(forElement) {
    var useDefault = false;
    var forceAction = false;
    var menuTarget = null;
    const contextMenuParent = document.getElementById("contextMenuParent");
    const contextMenu = document.getElementById("contextMenu");

    // Because we're destroying everything on every update, we need to find our menu target again
    function updateMenuTarget(){
        if(!menuTarget){return}
        console.log("Update Menu");
        var newParent = document.getElementById(menuTarget.parentElement.id)
        var menuIndex = Array.from(menuTarget.parentElement.children).indexOf(menuTarget)
        menuTarget = newParent.children[menuIndex];

        {
            const assignmentNum = menuTarget.cellIndex - 1;
            const studentRow = menuTarget.parentElement.rowIndex - 1;
            const studentName = menuTarget.parentElement.firstChild.textContent;
            const content = []
            contextMenu.classList = []
            if(studentRow >= 0){
                content.push(studentName, `Row: ${studentRow}`)
                contextMenu.classList.add("has-student")
                if(studentRow == 0){
                    contextMenu.classList.add("first-student")
                }
                if(studentRow == tableState.Contents.length - 1){
                    contextMenu.classList.add("last-student")
                }
            }
            if(assignmentNum > 0){
                content.push(`Assignment: ${assignmentNum}`)
                contextMenu.classList.add("has-assignment")
            }

            const title = contextMenu.querySelector(".title")
            if(!debugMode)
            {
                title.style.display = "none";
            }
            title.textContent = content.join(" - ")

            var assignmentElements = contextMenu.getElementsByClassName("assignment");
            var removeElementsLists = Array.from(assignmentElements).map((element) => Array.from(element.getElementsByClassName("remove")))
            var removeElements = Array.prototype.concat(...removeElementsLists)
            const assignmentIndex = assignmentNum - 1;
            for(element of removeElements){
                function updateElement(isEmpty, position, index){
                    var action = isEmpty ? "Remove" : "Clear";
                    if(forceAction){
                        action = "Remove"
                    }
                    element.textContent = `${action} ${position}`
                    if(isEmpty) {
                        element.classList.add("is-empty")
                    }
                    else {
                        element.classList.remove("is-empty")
                    }
                    if(index < 0 | index >= tableState.NumAssignments){
                        element.classList.add("hidden-column")
                    }
                    else {
                        element.classList.remove("hidden-column")
                    }
                }
                if(element.matches('.up')){
                    const isEmpty = isColumnEmpty(assignmentIndex - 1)
                    updateElement(isEmpty, "Assignment Left", assignmentIndex - 1)
                    
                }
                if(element.matches('.self')){
                    const isEmpty = isColumnEmpty(assignmentIndex)
                    updateElement(isEmpty, "This Assignment", assignmentIndex)
                }
                if(element.matches('.down')){
                    const isEmpty = isColumnEmpty(assignmentIndex+1);
                    updateElement(isEmpty, "Assignment Right", assignmentIndex+1)
                }
            }
        }
    }

    contextMenu.addEventListener('click', handleClicks);
    function handleClicks(event){
        if (event.target.matches('.remove')) {
            const currRow = menuTarget.parentElement.rowIndex - 1;
            const currCol = menuTarget.cellIndex - 2;

            var currIndex = currRow;
            var remove = removeRow;
            if(event.target.matches('.student')){
                remove = removeRow;
                currIndex = currRow;
            }
            if(event.target.matches('.assignment')){
                remove = removeColumn.bind(null, forceAction);
                currIndex = currCol;
            }
            
            if(event.target.matches('.up')){
                
                remove(currIndex - 1)
            }
            if(event.target.matches('.self')){
                remove(currIndex)
                closeMenu()
            }
            if(event.target.matches('.down')){
                remove(currIndex + 1)
            }
            updateMenuTarget()
        }
        if (event.target.matches('.add')) {
            const currRow = menuTarget.parentElement.rowIndex - 1;
            const currCol = menuTarget.cellIndex - 2;

            var currIndex = currRow;
            var add = createStudent;
            if(event.target.matches('.student')){
                add = createStudent;
                currIndex = currRow;
            }
            if(event.target.matches('.assignment')){
                add = addColumn.bind(null, "");
                currIndex = currCol;
            }
            

            if(event.target.matches('.up')){
                add(currIndex)
            }
            if(event.target.matches('.down')){
                add(currIndex + 1)
            }
            updateMenuTarget()

        }
    }
    function editMenu(x, y, hidden, parent = contextMenu.parentElement){
        if(!!contextMenu){
            contextMenu.style.display = hidden ? "none" : "block";
            contextMenu.hidden = hidden;

            //console.log(y, contextMenu.clientHeight, window.innerHeight );
            const bottomPadding = 25;
            if(y + contextMenu.clientHeight > (window.innerHeight - bottomPadding) ) {
                y = y - (y + contextMenu.clientHeight - window.innerHeight) - bottomPadding;
            }
            contextMenu.style.top = `${y + window.scrollY}px`;
            contextMenu.style.left = `${x}px`;
            parent.appendChild(contextMenu)
        }
    }
    function closeMenu(){
        hideMenu()
        menuTarget = null
    }
    function hideMenu(){
        editMenu(0, 0, true)
    }
    function showMenu(x, y, onElement){
        editMenu(x, y, false, onElement)
    }
    function contextCeption(event){
        menuTarget.focus()
        hideMenu();
        return true;
    }
    function callContextMenu(event){
        //console.log("context click");
        event.preventDefault();
        menuTarget = event.target
        
        updateMenuTarget()

        showMenu(event.x, event.y, menuTarget.parentElement);
    }
    function shouldForceMenu(event){
        forceAction = event.shiftKey;
        updateMenuTarget()
    }

    hideMenu()
    document.addEventListener("keydown", shouldForceMenu);
    document.addEventListener("keyup", shouldForceMenu);
    forElement.addEventListener("contextmenu", callContextMenu);
    contextMenu.addEventListener("contextmenu", contextCeption);
}

function start() {
    console.log("Starting")

    // Check if there's any local storage
    if(!!localStorage.getItem("data")){
        loadTable();
    }
    else{
        resetTable();
        saveTable();
    }

    activateContextMenu(tableDOM);

    document.addEventListener("keydown", (event) => {
        if(event.key.toLowerCase() == "z"){
            if(event.ctrlKey | event.metaKey){
                if(event.shiftKey) {//Redo, heck that
                }
                else {//Undo
                    reloadToLastHistory()
                }

            }
        }
    })

    // const observer = new MutationObserver((mutationRecords) => {
        
    //     const target = mutationRecords[0].target;
    //     console.log(target.data)
    //     //target.data = Number.parseInt(target.data);
        
    //     updateData()
    // })
    // console.log(tableDOM)
    // observer.observe(tableDOM, {
    // characterData: true,
    // subtree: true,
    // })

    regenerateTableDOM()
}
resetTable()
start()