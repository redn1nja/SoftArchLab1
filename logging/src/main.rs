#[macro_use] extern crate rocket;


use uuid::Uuid;
use rocket::serde::{Deserialize, json::Json};
use dashmap::DashMap;
use rocket::State;

#[derive(Deserialize)]
#[serde(crate = "rocket::serde")]
struct Message<'r> {
    uuid: &'r str,
    msg: &'r str,
}

struct Counter{
    data: DashMap<Uuid, String>,
}

#[post("/", format="application/json", data = "<msg>")]
fn post(msg: Json<Message<'_>>, state: &State<Counter>) -> String {
    state.data.insert(Uuid::parse_str(msg.uuid).unwrap(), msg.msg.to_string());
    format!("{}: {}", msg.uuid, msg.msg)
}

#[get("/")]
fn get(state: &State<Counter>) -> String {
    let mut result = String::new();
    for (_, v) in state.data.clone().into_iter(){
        result.push_str(&format!("{}\n", v));
    }
    result
}

#[launch]
fn rocket() -> _ {
    rocket::build().manage(Counter{data: DashMap::new()}).mount("/", routes![post, get])
}
