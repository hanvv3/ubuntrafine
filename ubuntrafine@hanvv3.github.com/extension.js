const { St, Clutter, GObject, GLib } = imports.gi;
const Main = imports.ui.main;
const PanelMenu = imports.ui.panelMenu;
const PopupMenu = imports.ui.popupMenu;
const Slider = imports.ui.slider;

var BrightnessControl = GObject.registerClass(
class BrightnessControl extends PanelMenu.Button {
    _init() {
        super._init(0.0, 'Brightness Control');
        
        let icon = new St.Icon({
            icon_name: 'display-brightness-symbolic',
            style_class: 'system-status-icon brightness-control-icon'
        });
        this.add_child(icon);
        
        // Align dropdown slider container with the icon in the center.
        this.menu._arrowAlignment = 0.5;

        let currentValue = this._getCurrentBrightness();
        this._slider = new Slider.Slider(currentValue);
        this._sliderItem = new PopupMenu.PopupBaseMenuItem({
            activate: false,
            hover: false,
            can_focus: false
        });
        this._sliderItem.add_child(this._slider.actor);
        this._slider.connect('notify::value', this._sliderChanged.bind(this));
        this.menu.addMenuItem(this._sliderItem);

        this.menu.connect('open-state-changed', this._onMenuToggled.bind(this));
        this.menu.passEvents = true;
        this.menu.actor.connect('button-press-event', this._onMenuActorPressed.bind(this));
    }

    _getCurrentBrightness() {
        let [result, out, err, exit] = GLib.spawn_command_line_sync("env TERM=xterm ufctl get");
        if (exit === 0) {
            let match = out.toString().match(/brightness:\s+(\d+)/);
            if (match && match[1]) {
                let brightness = parseInt(match[1], 10);
                return brightness / 100;
            }
        }
        return 0.5; // Default value if brightness cannot be determined
    }

    _onMenuToggled(menu, open) {
        if (open) {
            // Update the slider value when the menu is opened
            this._slider.value = this._getCurrentBrightness();
        }
    }

    _onMenuActorPressed(actor, event) {
        if (this.menu.isOpen) {
            this.menu.close();
        } else {
            this.menu.open();
        }
        return Clutter.EVENT_STOP;
    }

    _sliderChanged() {
        let brightness = Math.round(this._slider.value * 100);
        let commandLine = `env TERM=xterm ufctl set 0 ${brightness}`;
    
        try {
            let [result, out, err, exit] = GLib.spawn_command_line_sync(commandLine);
            if (exit !== 0) {
                log(`Failed to run command: ${commandLine}. Error: ${err.toString()}`);
            } else {
                log(`Command ran successfully: ${commandLine}. Output: ${out.toString()}`);
            }
        } catch (e) {
            log(`Exception while running command: ${commandLine}. Message: ${e.message}`);
        }
    }
});

let _brightnessControl;

function init() {}

function enable() {
    _brightnessControl = new BrightnessControl();
    Main.panel.addToStatusArea('brightness-control', _brightnessControl, 0, 'right');
}

function disable() {
    _brightnessControl.destroy();
}
