export interface Args {
    params: { id: string },
    request: Request
}

export enum Method {GET, POST, PUT, DELETE}

export class Endpoint {
    [id: string]: (args: Args) => Response;
}

export const endpoints = Array.from({length: Object.keys(Method).length / 2}, () => new Endpoint());
export const GET = endpoints[Method.GET];
export const POST = endpoints[Method.POST];
export const PUT = endpoints[Method.PUT];
export const DELETE = endpoints[Method.DELETE];

export function endpoint(method: Method, args: Args) {
    const handler = endpoints[method][args.params.id];
    return handler ? handler(args) : new Response(null, {status: 404});
}


