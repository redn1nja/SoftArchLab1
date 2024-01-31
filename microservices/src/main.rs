#[macro_use] extern crate rocket;


#[post("/", format="plain", data = "<msg>")]
fn post(msg: &str) -> String {
    let s = std::fmt::format(format_args!("Hello, {}!", msg));
    s
}

#[launch]
fn rocket() -> _ {
    rocket::build().mount("/", routes![post])

}
