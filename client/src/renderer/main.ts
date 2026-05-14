import { ipcRenderer, IpcRendererEvent } from "electron"

const startButton = document.getElementById("start") as HTMLButtonElement
startButton.addEventListener("click", () => {
  ipcRenderer.send("start")
})

const cs2Button = document.getElementById("inject") as HTMLButtonElement
cs2Button.textContent = "Start CS2"
cs2Button.addEventListener("click", () => {
  ipcRenderer.send("startCS2")
})

const imageElem = document.getElementById("image") as HTMLImageElement

// Status display
const statusEl = document.getElementById("status") || document.createElement("div")
statusEl.id = "status"
statusEl.style.cssText = "color:#0f0;font-family:monospace;padding:8px;min-height:20px"
if (!document.getElementById("status")) {
  const titleInput = document.getElementById("title")
  if (titleInput && titleInput.parentNode) {
    titleInput.parentNode.insertBefore(statusEl, titleInput)
  }
}

ipcRenderer.on("status", (_event: IpcRendererEvent, msg: string) => {
  statusEl.textContent = msg
  console.log("[Status]", msg)
})

ipcRenderer.on("osrImage", (event: IpcRendererEvent, arg: { image: string }) => {
  const { image } = arg
  imageElem.src = image
})

ipcRenderer.on("gameEvent", (event: IpcRendererEvent, arg: { eventName: string; eventData: string }) => {
  console.log(`[GameEvent] ${arg.eventName}: ${arg.eventData}`)
  // MVP animation will be triggered here when round_mvp is received
  if (arg.eventName === "round_mvp") {
    const data = JSON.parse(arg.eventData)
    console.log(`[MVP] userid=${data.userid} reason=${data.reason}`)
  }
})

window.onfocus = function() {
  console.log("focus")
}
window.onblur = function() {
  console.log("blur")
}
