const { timers } = require('jquery')

window.$ = window.jQuery = require('jquery')

var modes = ["solid", "wave", "trail", "morse"]
var current_mode = "solid"
var old_mode = null
var cToArduinoStatus = false
var cToServerStatus = false
var javaScriptToServerStatus = false
var on = true;
var intensity = 100;

var socket = io.connect('http://localhost:3000')
socket.emit("message", "vj")

socket.on("message", ProcessMessage);

function deactivate() {
    if (current_mode != null) {
        $("." + current_mode).removeClass("active")
    }
    current_mode = null
}

function cut_out_non_numerals(str_to_set) {
    var str = str_to_set
    var checking = true
    while (checking) {
        var invalid_char_exists = false
        for (var i = 0; i < str.length; i++) {
            var char = str.charAt(i)
            if (char != "0" &&
                char != "1" &&
                char != "2" &&
                char != "3" &&
                char != "4" &&
                char != "5" &&
                char != "6" &&
                char != "7" &&
                char != "8" &&
                char != "9" &&
                char != ".") { //123a4s5d.123a3a

                var lower = str.substring(0, i);
                var upper = ""
                if (i != str.length - 1) {

                    upper = str.substring(i + 1, str.length)
                }
                str = lower + upper
                invalid_char_exists = true
                break
            }
        }
        if (!invalid_char_exists) checking = false
    }
    if (str.indexOf(".") != -1) {
        var substr = str.substring(str.indexOf(".") + 1, str.length)
        substr = substr.split(".").join("")
        str = str.substring(0, str.indexOf(".") + 1) + substr
    }
    else str += ".0"
    if (str.indexOf(".") == str.length - 1) str += 0
    return str
}

function cut_out_non_morse(str_to_set) {
    var str = str_to_set
    var checking = true
    while (checking) {
        var invalid_char_exists = false
        for (var i = 0; i < str.length; i++) {
            var char = str.charAt(i)
            if (char != "." && char != "-" && char != " ") { //...---... ...---...

                var lower = str.substring(0, i);
                var upper = ""
                if (i != str.length - 1) {

                    upper = str.substring(i + 1, str.length)
                }
                str = lower + upper
                invalid_char_exists = true
                break
            }
        }
        if (!invalid_char_exists) checking = false
    }
    return str
}

$(document).ready(function() {
    setInterval(update, 50)

    $(".power").click(function() {
        on = !on;
        if (on) {   
            $(".power").css("background-color", "var(--green)")
            $(".content").css("opacity", "100%")
            $(".content").find('*').removeAttr("disabled")
            socket.emit("message", "cmessage-\x01")
        }
        else {            
            $(".power").css("background-color", "var(--red)")
            $(".content").css("opacity", "25%")
            $(".content").find('*').attr("disabled", true)
            socket.emit("message", "cmessage-\0")
        }
    })
    $(".intensity").click(function() {
        if ($(this).hasClass("ir")) intensity += intensity < 100 ? 10 : 0
        else intensity -= intensity > 0 ? 10 : 0;
        var asStr =  ((intensity == 100 || intensity == 0) ? (intensity == 100 ? "" : "00") : "0") + String(intensity)
        socket.emit("message", "cmessage-i" + asStr)
    })

    $(".rgb").on("change paste keyup", function() {
        deactivate()
    })
    $("#wave-rate-selector").on("change paste", function() {
        var old = $(this).val()
        var news = cut_out_non_numerals(old)
        if (news != old) $(this).val(news)
        deactivate()
    })
    $("#trail-speed").on("change paste", function() {
        var old = $(this).val()
        var news = cut_out_non_numerals(old)
        if (news != old) $(this).val(news)
        deactivate()
    })
    $("#trail-color").on("change", function() {
        deactivate()
    })
    $("#morse-text").on("change paste", function() {
        var old = $(this).val()
        var news = cut_out_non_morse(old)
        if (news != old) $(this).val(news)
        deactivate()
    })
    $("#morse-dot-ms").on("change paste", function() {
        var old = $(this).val()
        var news = cut_out_non_numerals(old)
        if (news != old) $(this).val(news)
        deactivate()
    })
    $("#morse-dash-ms").on("change paste", function() {
        var old = $(this).val()
        var news = cut_out_non_numerals(old)
        if (news != old) $(this).val(news)
        deactivate()
    })
    $("#morse-space-ms").on("change paste", function() {
        var old = $(this).val()
        var news = cut_out_non_numerals(old)
        if (news != old) $(this).val(news)
        deactivate()
    })
    $("#morse-space-letter-ms").on("change paste", function() {
        var old = $(this).val()
        var news = cut_out_non_numerals(old)
        if (news != old) $(this).val(news)
        deactivate()
    })
    $(".trail-direction").click(function() {
        if ($(this).text() == "Direction: out (toggle)") {
            $(this).text("Direction: in (toggle)")
        }
        else $(this).text("Direction: out (toggle)")
        deactivate()
    })

    $(".use").click(function() {
        if ($(this).hasClass("solid")) {
            
            if (current_mode != "solid") {
                
                if ($(".rgb").val() != "") {
                    
                    socket.emit("message", "cmessage-" + "s" + $(".rgb").val())

                    if (current_mode != null) $("." + current_mode).removeClass("active")
                    $(this).addClass("active")
                    current_mode = "solid"
                }
            }
        }
        else if ($(this).hasClass("wave")) {

            if (current_mode != "wave") {

                if ($("#wave-rate-selector").val() != "") {

                    socket.emit("message", "cmessage-" + "w" + $("#wave-rate-selector").val() + ":")

                    if (current_mode != null) $("." + current_mode).removeClass("active")
                    $(this).addClass("active")
                    current_mode = "wave"
                }
            }
        }
        else if ($(this).hasClass("trail")) {
            
            if (current_mode != "trail") {

                if ($("#trail-speed").val() != "") {

                    socket.emit("message", "cmessage-" +
                                           "t" +
                                           $("#trail-speed").val() + ":" +
                                           $("#trail-color").val() + ":" +
                                           ($(".trail-direction").text() == "Direction: out (toggle)" ? "1" : "0"))

                    if (current_mode != null) $("." + current_mode).removeClass("active")
                    $(this).addClass("active")
                    current_mode = "trail"
                }
            }
        }
        else if ($(this).hasClass("morse")) {

            if (current_mode != "morse") {

                if ($("#morse-text").val() != "" &&
                    $("#morse-dot-ms").val() != "" &&
                    $("#morse-dash-ms").val() != "" &&
                    $("#morse-space-ms").val() != "" &&
                    $("#morse-space-letter-ms").val() != "") {

                    socket.emit("message", "cmessage-" +
                                   "m" +
                                   $("#morse-text").val() + ":" +
                                   $("#morse-dot-ms").val() + ":" +
                                   $("#morse-dash-ms").val() + ":" +
                                   $("#morse-space-ms").val() + ":" +
                                   $("#morse-space-letter-ms").val() + ":")

                    if (current_mode != null) $("." + current_mode).removeClass("active")
                    $(this).addClass("active")
                    current_mode = "morse"
                }
            }
        }
    })
})

function ProcessMessage(msg) {
    switch (msg.slice(0, 1)) {
        case "z":
            var m = msg.slice(2, msg.length)
            //connection change
            switch (msg.slice(1, 2)) {
                case "g":
                    if (m == "cs") cToServerStatus = true;
                    else if (m == "ca") cToArduinoStatus = true;
                    else if (m == "js") javaScriptToServerStatus = true;
                    break
                case "b":
                    if (m == "cs") cToServerStatus = false;
                    else if (m == "ca") cToArduinoStatus = false;
                    else if (m == "js") javaScriptToServerStatus = false;
                    break
                default:
                    console.log("Invalid g/b")
                    break
            }
            break
        default:
            break
    }
}

function update() {
    if (cToArduinoStatus) {
        if ($(".c_a_conn").hasClass("bad")) {
            $(".c_a_conn").removeClass("bad")
            $(".c_a_conn").addClass("good")
            $(".c_a_conn").text("Connected")
        }
    }
    else {
        if ($(".c_a_conn").hasClass("good")) {
            $(".c_a_conn").removeClass("good")
            $(".c_a_conn").addClass("bad")
            $(".c_a_conn").text("Disconnected")
        }
    }
    if (cToServerStatus) {
        if ($(".c_conn").hasClass("bad")) {
            $(".c_conn").removeClass("bad")
            $(".c_conn").addClass("good")
            $(".c_conn").text("Connected")
        }
    }
    else {
        if ($(".c_conn").hasClass("good")) {
            $(".c_conn").removeClass("good")
            $(".c_conn").addClass("bad")
            $(".c_conn").text("Disconnected")
        }
    }
    if (javaScriptToServerStatus) {
        if ($(".js_conn").hasClass("bad")) {
            $(".js_conn").removeClass("bad")
            $(".js_conn").addClass("good")
            $(".js_conn").text("Connected")
        }
    }
    else {
        if ($(".js_conn").hasClass("good")) {
            $(".js_conn").removeClass("good")
            $(".js_conn").addClass("bad")
            $(".js_conn").text("Disconnected")
        }
    }
}