async function main(cart) {
    platform_init();
    log("Loaded WebGL2 graphics");

    await platform_sleep_ms(100); // Wait for cart to load

    let nes = new nes6502();   
}