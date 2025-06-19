import {defineCollection} from "astro:content";
import {glob} from "astro/loaders";

const wiki = defineCollection({
    loader: glob({pattern: '**/*.md', base: '../wiki'})
});

export const collections = {wiki};