/**
 * Just playing around
 */
const doc = (() => {
    const d = document;
    let context = {
        finder:null,
        element:null,
        elements:[],
    };
    let finder = {
        id:(id) => {
            context.element = d.getElementById(id);
            context.elements[0] = context.element;
            return context.finder;
        },
        hasText:(text) => {
            let found = false;
            const getText = (txt)=>{
                if(!txt){return false;}
                return txt.toString().toLowerCase().includes(
                    text.toString().toLowerCase()
                )
            }
            if(context.element){
                if(context.elements.length > 1) {
                    context.elements.forEach(
                        (e)=>{
                            if(e && !found){
                                let txt = e.textContent || e.innerText;
                                found = getText(txt)
                            }
                        }
                    );
                    return found;
                }
                let txt = context.element.textContent || context.element.innerText;
                found = getText(txt);
            }
            // console.log(`Cannot find element.hasText("${text}") because an element has not been set. A document element context is set by using elements.id().`)
            return found;
        },
        tag:(name) => {
            let tags = d.getElementsByTagName(name);
            if(tags) {
                context.element = tags[0];
                context.elements = tags;
            }
            return context.finder;
        },
        class:(selector)=>{
            let classes = d.getElementsByClassName(selector);
            if(classes) {
                context.element = classes[0];
                context.elements = classes;
            }
            return context.finder;
        },
        query:(query)=>{
            return context.finder;
        }
    };
    return context.finder = finder;
})();

const Search = (()=>{
    let searchContext = {
        sections: {
        
        }
    }
    
    let simpleSearch = (text)=>{
        let e = elements;
        let input, filter, ul, li, a, i, txtValue;
        input =     doc.id('searchController');
        filter =    input.value.toUpperCase();
        ul =        doc.id("content");
        li =        doc.tag('div');
    
        // Loop through all list items, and hide those who don't match the search query
        for (i = 0; i < li.length; i++) {
            a = li[i].getElementsByTagName("a")[0];
            txtValue = a.textContent || a.innerText;
            if (txtValue.toUpperCase().indexOf(filter) > -1) {
                li[i].style.display = "";
            } else {
                li[i].style.display = "none";
            }
        }
    }
    return {
        submit: (text)=>{
            // $('input#searchController')
            //     .quicksearch(
            //         'div h2 h3 h4 table tbody tr pre'
            //     );
        }
    }
})();
