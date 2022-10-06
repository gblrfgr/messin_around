// webkit will be the death of us all
window.requestFileSystem = window.requestFileSystem || window.webkitRequestFileSystem;
window.directoryEntry = window.directoryEntry || window.webkitDirectoryEntry;

// alison bechdel moment
const divsToWatchOutFor = ["uhHOwf BYbUcd", "bRMDJf islir", "LicuJb uhHOwf BYbUcd", "ZGomKf"];
const imagesToLookAt = ["n3VNCb KAlRDb", "rHLoR"];

function replaceImages(mutationList, observer) {
    for (let className of divsToWatchOutFor) {
        for (let image of document.getElementsByClassName(className)) {
            image.children[0].src = "https://pagesix.com/wp-content/uploads/sites/3/2020/11/Phil-Collins-Orianne-Cevey.jpg";
        }
    }

    for (let className of imagesToLookAt) {
        for (let image of document.getElementsByClassName(className)) {
            image.src = "https://pagesix.com/wp-content/uploads/sites/3/2020/11/Phil-Collins-Orianne-Cevey.jpg";
        }
    }
}

const observedNode = document.body;
const observerOptions = {
    childList: true,
    subtree: true,
};

const observer = new MutationObserver(replaceImages);
observer.observe(document.body, observerOptions);

replaceImages(); // runs on page load