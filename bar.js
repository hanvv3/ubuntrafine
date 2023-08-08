const { St, Clutter, GObject } = imports.gi;
const Main = imports.ui.main;
const PanelMenu = imports.ui.panelMenu;
const PopupMenu = imports.ui.popupMenu;
const Slider = imports.ui.slider;

var BrightnessControl = GObject.registerClass(
class BrightnessControl extends PanelMenu.Button {
    _init() {
        super._init(0.0, 'Brightness Control');
        
        // set an icon
        let icon = new St.Icon({
            icon_name: 'display-brightness-symbolic',  // display icon. you need to rename it correctly.
            style_class: 'system-status-icon'
        });
        this.add_child(icon);
        

        // add slider
        let currentValue = this._getCurrentBrightness();
        this._slider = new Slider.Slider(currentValue);
        this._sliderItem = new PopupMenu.PopupBaseMenuItem({
            activate: false,
            hover: false,
            can_focus: false
        });
        this._sliderItem.add_child(this._slider);
        this._slider.connect('notify::value', this._sliderChanged.bind(this))
        //this._slider.connect('value-changed', this._onValueChanged.bind(this));
        this.menu.addMenuItem(this._sliderItem);
        
        // keeps menu from automatic roll down.
        this.menu.passEvents = true;
        this.menu.actor.connect('button-press-event', this._onMenuActorPressed.bind(this));
    }

    _getCurrentBrightness() {
        let [result, out, err, exit] = GLib.spawn_command_line_sync("/path/to/your/executable --get-brightness");
        if (exit === 0) {
            let brightness = parseFloat(out.toString());
            return brightness / 100;  // Convert to value between 0 and 1 for the slider
        }
        return 0.5;  // Default value if unable to get current brightness
    }

    _onMenuActorPressed(actor, event) {
        if (this.menu.isOpen) {
            this.menu.close();
        } else {
            this.menu.open();
        }
        return Clutter.EVENT_STOP;
    }

    _onValueChanged(slider) {
        // Code to change the brightness here.
        // You could call out to your C++ program with GLib.spawn_command_line_async()
    
        // Convert the slider value (0 to 1) to a brightness percentage (0 to 100)
        let brightness = Math.round(this._slider.value * 100);
        // Define the path to the executable
        let executablePath = "~/Git-Apps/LG_ultrafine_display_brightness/lg_controller";
        // Build the command line
        let commandLine = executablePath + " --set-brightness " + brightness;
    
        try {
            // Run the command
            imports.gi.GLib.spawn_command_line_async(commandLine);
        } catch (e) {
            // If the command fails, log the error
            logError(e, "Failed to run command: " + commandLine);
        }
    }
});

let _brightnessControl;

function init() {
}

function enable() {
    _brightnessControl = new BrightnessControl();
    Main.panel.addToStatusArea('brightness-control', _brightnessControl, 0, 'right');
}

function disable() {
    _brightnessControl.destroy();
}




