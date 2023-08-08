const { St, Clutter, GObject } = imports.gi;
const Main = imports.ui.main;
const PanelMenu = imports.ui.panelMenu;
const PopupMenu = imports.ui.popupMenu;
const Slider = imports.ui.slider;

var BrightnessControl = GObject.registerClass(
class BrightnessControl extends PanelMenu.Button {
    _init() {
        super._init(0.0, 'Brightness Control');
        
        // 아이콘 설정
        let icon = new St.Icon({
            icon_name: 'display-brightness-symbolic',  // 디스플레이 아이콘. 이름을 적절하게 변경해야 할 수 있습니다.
            style_class: 'system-status-icon'
        });
        this.add_child(icon);
        
        // 슬라이더 추가
        this._slider = new Slider.Slider(0.5);  // 초기값을 50%로 설정
        this._sliderItem = new PopupMenu.PopupBaseMenuItem({
            activate: false,
            hover: false,
            can_focus: false
        });
        this._sliderItem.add_child(this._slider);
        this._slider.connect('value-changed', this._onValueChanged.bind(this));
        this.menu.addMenuItem(this._sliderItem);
        
        // 메뉴가 자동으로 열리지 않도록 설정
        this.menu.passEvents = true;
        this.menu.actor.connect('button-press-event', this._onMenuActorPressed.bind(this));
    }
    
    _onMenuActorPressed(actor, event) {
        if (this.menu.isOpen) {
            this.menu.close();
        } else {
            this.menu.open();
        }
        return Clutter.EVENT_STOP;
    }

    _onValueChanged(slider, value) {
        // Code to change the brightness here.
        // You could call out to your C++ program with GLib.spawn_command_line_async()
    
        // Convert the slider value (0 to 1) to a brightness percentage (0 to 100)
        let brightness = Math.round(value * 100);
    
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




