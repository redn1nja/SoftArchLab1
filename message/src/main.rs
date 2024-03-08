#[macro_use] extern crate rocket;

#[get("/")]
fn hello() -> &'static str {
    "No message"
}

#[launch]
fn rocket() -> _ {
    rocket::build().configure(rocket::Config::figment().merge(("port", 5001))).mount("/", routes![hello])
}