const char *topics[] = {
    "Sports",      "Technology",  "Music",    "Movies",      "Politics",
    "Health",      "Science",     "Art",      "Travel",      "Education",
    "Finance",     "Environment", "Food",     "Fashion",     "History",
    "Gaming",      "Literature",  "Business", "Automobiles", "Fitness",
    "Space",       "Photography", "Nature",   "Psychology",  "Religion",
    "Programming", "Comedy",      "DIY",      "Parenting",   "Economics",
    "Philosophy",  "Startups"};

const char *messages[] = {
    "The sun sets over the horizon, painting the sky with hues of orange, "
    "pink, "
    "and purple. Each day ends with a masterpiece created by nature.",
    "Artificial intelligence continues to evolve, challenging our "
    "understanding "
    "of creativity, intelligence, and the boundaries of human potential.",
    "The thrill of a last-minute goal in a championship game is unmatched. "
    "It's "
    "a moment of sheer exhilaration for players and fans alike.",
    "In a bustling city, the hum of traffic, distant chatter, and the rhythm "
    "of "
    "footsteps create a symphony of urban life.",
    "A steaming cup of coffee on a rainy day brings comfort and warmth, a "
    "simple "
    "pleasure that soothes the soul.",
    "The mysteries of black holes intrigue scientists and inspire "
    "storytellers. "
    "What lies beyond their event horizons remains one of the universe's "
    "greatest secrets.",
    "Music has the power to evoke emotions, from joy to sorrow, love to "
    "nostalgia. "
    "It is a universal language that transcends borders and cultures.",
    "The aroma of fresh bread baking in the oven fills the house with a sense "
    "of "
    "home and happiness.",
    "Exploring ancient ruins offers a glimpse into civilizations long gone, "
    "each "
    "stone whispering tales of the past.",
    "The crunch of autumn leaves underfoot is a small but satisfying pleasure, "
    "marking the arrival of a crisp, colorful season.",
    "In the face of a storm, trees bend but do not break, a lesson in "
    "resilience "
    "and adaptability.",
    "Space exploration fuels our curiosity, igniting dreams of walking on Mars "
    "or discovering new worlds beyond our solar system.",
    "A dog’s wagging tail is a universal sign of happiness, a reminder of the "
    "joy "
    "that animals bring into our lives.",
    "As coding languages evolve, the challenge of keeping up with new "
    "frameworks "
    "and paradigms keeps developers on their toes.",
    "The gentle hum of a vinyl record spinning on a turntable brings "
    "nostalgia, "
    "reminding us of simpler times.",
    "Sunflowers follow the sun across the sky, a symbol of positivity and "
    "hope, "
    "teaching us to seek the light.",
    "The intricate designs of mandalas, carefully drawn by hand, represent "
    "balance, harmony, and mindfulness.",
    "Each wave that washes ashore erases footprints, a fleeting reminder of "
    "the "
    "impermanence of life.",
    "In the deep blue of the ocean, coral reefs teem with life, their vivid "
    "colors "
    "a testament to nature’s artistry.",
    "A handwritten letter carries a personal touch, a rare treasure in an era "
    "dominated by digital communication.",
    "The first snowfall of winter transforms the world into a serene, white "
    "wonderland, "
    "softening edges and silencing noise.",
    "Stargazing on a clear night fills us with wonder, as constellations weave "
    "myths and stories across the heavens.",
    "The sound of a train whistle in the distance evokes nostalgia, "
    "symbolizing "
    "adventure, travel, and the passage of time.",
    "The crunch of gravel under bicycle tires on a countryside trail connects "
    "us "
    "with nature and a simpler way of life.",
    "Every sunrise is a new beginning, an opportunity to chase dreams and make "
    "the most of the day ahead.",
    "The pages of an old book carry more than words; they carry history, "
    "memories, and the faint scent of time.",
    "A winding mountain road challenges the spirit of adventure, promising "
    "breathtaking views at the summit.",
    "The soft glow of fireflies on a summer night creates a magical ambiance, "
    "filling the air with wonder and delight.",
    "The laughter of children at play is a reminder of the innocence and joy "
    "that "
    "exist in the simplest moments of life.",
    "A library is a sanctuary of knowledge, where every book opens a door to a "
    "different world, filled with stories, facts, and imagination.",
    "The smell of rain hitting dry earth, known as petrichor, is one of "
    "nature's "
    "most refreshing and nostalgic scents.",
    "The crackling of a campfire under a starry sky brings warmth, comfort, "
    "and a "
    "sense of connection to the wilderness.",
    "Each ring on a tree trunk tells the story of a year, a natural diary of "
    "growth "
    "and survival through the seasons.",
    "The beauty of a handwritten journal lies in its imperfections, each word "
    "reflecting the thoughts and emotions of the writer.",
    "A shooting star streaking across the night sky is a fleeting moment of "
    "wonder, "
    "a reminder to cherish the small, magical moments in life.",
    "Baking cookies fills a home with an aroma that invites smiles and creates "
    "memories of family gatherings and holidays.",
    "The sound of ocean waves rolling onto the shore is a soothing rhythm that "
    "calms "
    "the mind and washes away worries.",
    "A walk through a sunflower field is like stepping into a painting, "
    "surrounded by "
    "vivid yellows and greens under a bright blue sky.",
    "The intricate patterns of frost on a windowpane are a reminder that even "
    "the coldest "
    "days hold beauty if you look closely.",
    "The hum of a busy café, with clinking cups and quiet chatter, creates a "
    "backdrop for "
    "creativity and conversation.",
    "When thunder rolls through the sky, it's as though nature is reminding us "
    "of its "
    "untamed power and majesty.",
    "A cat curling up in your lap is a moment of pure peace, their soft "
    "purring acting as "
    "a lullaby for the soul.",
    "The art of origami teaches us patience and precision, as simple folds "
    "transform paper "
    "into creatures and shapes of wonder.",
    "On a foggy morning, the world feels like a mysterious painting, shrouded "
    "in mist and "
    "inviting exploration.",
    "Watching a flock of birds take flight in perfect unison is a display of "
    "nature's harmony "
    "and instinctual choreography.",
    "The burst of flavor from a freshly picked fruit reminds us of the simple "
    "joys that "
    "nature provides.",
    "A handwritten postcard from a distant friend carries more emotion than a "
    "hundred "
    "digital messages ever could.",
    "As a river carves through rock over millennia, it reminds us of the quiet "
    "strength "
    "of persistence and time.",
    "The first bite of a favorite dish brings a wave of nostalgia, "
    "transporting us to "
    "moments and places we hold dear.",
    "The brilliance of fireworks lighting up the night sky is fleeting yet "
    "unforgettable, "
    "a celebration of life's vibrant moments.",
    "Walking barefoot on soft grass reconnects us to the earth, grounding our "
    "minds and "
    "refreshing our spirits.",
    "The rustle of leaves in a gentle breeze is nature's whisper, reminding us "
    "to slow "
    "down and listen.",
    "The glowing embers of a dying fire remind us that every end carries "
    "warmth and "
    "memories to cherish.",
    "Standing at the edge of a cliff, gazing at the expanse below, fills the "
    "soul with awe "
    "and a sense of infinite possibility.",
    "The clink of ice cubes in a glass of lemonade on a summer day is a small "
    "but iconic "
    "reminder of relaxation and refreshment.",
    "Watching the snowfall in silence creates a peaceful world, where every "
    "sound feels "
    "hushed and every moment timeless.",
    "A rainbow after a storm is a sign of hope, a vivid reminder that beauty "
    "often "
    "follows the darkest times.",
    "A well-played symphony in an opera house resonates in the hearts of "
    "listeners, each "
    "note carrying centuries of tradition and artistry.",
    "The quiet buzz of a honeybee among flowers is a sign of life’s "
    "interconnectedness, a "
    "delicate balance sustaining the planet."};
