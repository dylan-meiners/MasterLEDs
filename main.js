const electron = require('electron')
const { app, BrowserWindow } = require('electron')
const server = require('./src/js/server')

electron.Menu.setApplicationMenu(null)

function createWindow() {
    let win = new BrowserWindow({
        /*width: 1500,
        height: 800,
        x: 10,
        y: 10,*/
        width: 1500,
        height: 900,
        x: 1600,
        y: -400,
        webPreferences: {
            nodeIntegration: true
        }
    })

    win.loadFile('./src/html/index.html')

    //win.webContents.openDevTools()

    win.on('closed', () => {
        win = null
    })

    //var python_server_file = require('child_process').spawn('python', ['./src/py/client.py'])
}

app.on('ready', () => {
    createWindow()
})

app.on('window-all-closed', () => {
    if (process.platform != 'darwin') {
        app.quit()
    }
})

app.on('activate', () => {
    if (win === null) {
        createWindow()
    }
})