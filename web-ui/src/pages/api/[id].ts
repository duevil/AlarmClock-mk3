// noinspection JSUnusedGlobalSymbols

import {GET as _GET, POST as _POST, type Args, endpoints, endpoint, Method} from "./_endpoints.ts";


//#region endpoint definitions


_GET["foo"] = () => new Response("Hello Foo");

_GET["bar"] = () => new Response("Hello Bar");

_POST["foo"] = () => {
    console.log("Hello Foo");
    return new Response(null, {status: 204});
};

_POST["bar"] = () => {
    console.log("Hello Bar");
    return new Response(null, {status: 204});
};


//#endregion


export const prerender = false;
export const getStaticPaths = () => endpoints.flat().map(e => ({params: {id: e.id}}));
export const GET = (args: Args) => endpoint(Method.GET, args);
export const POST = (args: Args) => endpoint(Method.POST, args);
export const PUT = (args: Args) => endpoint(Method.PUT, args);
export const DELETE = (args: Args) => endpoint(Method.DELETE, args);