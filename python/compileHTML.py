
from collections import defaultdict
from copy import deepcopy
import re
from SafeEval import safe_eval
from helpers import normalise


def replaceInTemplateAtSpan(template: str, value: str, span: tuple[int, int]):
    return template[:span[0]] + value + template[span[1]:]

def calculateValue(calculation: str, operatorPattern: re.Pattern, keypairs: dict[str, str], evaluateResult = True) -> str:
    # Inside Calculations, the values change meaning from just strings to potential values
    value: str = calculation
    ##print(f"Calculation: {value}")
    # Safety Check, only evaluate if exists in a very limited subset of python
    operators = operatorPattern.search(value)
    if operators:
        normalLeft = normalise(operators['left'])
        normalRight = normalise(operators['right'])
        # Convert left and right to values, if possible
        left = keypairs[normalLeft] if normalLeft in keypairs else normalLeft
        right = keypairs[normalRight] if normalRight in keypairs else normalRight

        left = int(left) if left.isnumeric() else left
        right = int(right) if right.isnumeric() else right

        operator = normalise(operators['operator'])

        computation = ''.join([str(left), str(operator), str(right)])
        if evaluateResult:
            result = safe_eval(computation)
        else:
            result = deepcopy(computation)

        ##print(f"Result: {result}")

        return replaceInTemplateAtSpan(value, str(result), operators.span())

        # value = value[:operators.start()] + \
        #     str(safe_eval(computation)) + value[operators.end():]

    # If is a value that's been set before
    # WARNING!!! this could leak information about the build system
    if keypairs[value]:
        ##print(f"Old Value: {value}")
        return keypairs[value]
                


def createHTML(keypairs: dict[str, str], template: str, patterns: dict[str, re.Pattern]):
    keypairs = deepcopy(keypairs)
    # Is there a better way to reconstruct the template?
    commandPattern = patterns['command']
    calculatePattern = patterns['calculate']
    computePattern = patterns['compute']
    operatorPattern = patterns['operations']

    # First rebuild the template with any computations there may be
    todoCompute = computePattern.search(normalise(template))
    while todoCompute:
        value: str
        query: str
        result: str
        query, value, result = todoCompute['query'], todoCompute['value'], todoCompute['result']
        span = todoCompute.span()
        editedTemplate = False
        match(query):
            case "if":
                if(keypairs[value]):
                    template = replaceInTemplateAtSpan(template, result, span)
                    editedTemplate = True

            case "set":
                result = calculateValue(result, operatorPattern, keypairs)
                ##print(f"Setting: {value} to {result}")
                if keypairs[value]:
                    print(f"Overriding {keypairs[value]} with {result} at {value}")
                keypairs[value] = result;
            
            case "setraw":
                result = calculateValue(result, operatorPattern, keypairs, False)
                if keypairs[value]:
                    print(f"Overriding {keypairs[value]} with {result} at {value}")
                keypairs[value] = result;


            case "get":
                if keypairs[value]:
                    template = replaceInTemplateAtSpan(template, keypairs[value], span)
                    editedTemplate = True

            case "foreach":
                name, _, lst = value.split()
                if(keypairs[lst]):
                    out = [createHTML(defaultdict(str, [tuple(
                        [name.strip(), item.strip()])]), result, patterns) for item in keypairs[lst]]
                    template = template[:span[0]] + \
                        ",\n".join(out) + template[span[1]:]
                    editedTemplate = True
            case _:
                print(f"Unknown Query: {query}")
                pass

        if not editedTemplate:
            # Remove the failed case
            template = template[:span[0]] + template[span[1]:]

        todoCompute = computePattern.search(normalise(template))

    
    # Then build the template with the values
    match = commandPattern.search(template)
    while match:
        command = normalise(match["command"]);
        todoCalculate = calculatePattern.search(command)
        if todoCalculate:
            value = calculateValue(todoCalculate["calculation"], operatorPattern, keypairs)
        else:
            # WARNING!!! this could leak information about the build system
            value = keypairs[command]

        #print(value)

        template = replaceInTemplateAtSpan(template, str(value), match.span())
        match = commandPattern.search(template)
    return template.strip()
