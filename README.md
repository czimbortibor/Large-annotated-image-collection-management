# Large annotated image collection management
Images with metadata collected from RSS feeds with a [separate tool](https://github.com/czimbortibor/rss-atom-feedscraper) and saved to MongoDB.

Upon choosing a collection, the app tries to load everything into memory, while simultaneously running multiple hashing algorithms on the images.

Images hashes calculated, each applicable for a different use case:

    - Average hash
    - Perceptual hash
    - Marr Hildreth hash
    - Radial Variance hash
    - Block Mean hash
    - Color Moment hash


### The app

Instant reordering of the collection based on a selected picture and a hashing method, different layouts (grid, petal, spiral), larger sized image load on hover tooltip, displaying metadata and search based on some of its fields.

![Image](./screenshots/app_screenshot_grid.png)
![Image](./screenshots/app_screenshot_petals.png)
![Image](./screenshots/spiral_5000.png)
![Image](./screenshots/petal+tooltip.png)

#### Instant search and reorder
![Image](./screenshots/filter_GoT.png)

#### Metadata about the images
![Image](./screenshots/grid_metadata.png)
![Image](./screenshots/spiral+metadata.png)

#### 5000+ images
![Image](./screenshots/spiral_5000_zoomout.png)
![Image](./screenshots/grid_color.png)


### In-memory storage structure of the images
![Image](./screenshots/collection_struct.png)

