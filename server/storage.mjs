import storage from 'node-persist';

const storageInit = storage.init();


async function getPlayer(playerToken) {
  return getItem(`player/${playerToken}`);
}

async function setPlayer(playerToken, value) {
  return setItem(`player/${playerToken}`, value);
}

async function getItem(key) {
  await storageInit;
  return await storage.getItem(key);
}

async function setItem(key, value) {
  await storageInit;
  await storage.setItem(key, value);
}

export {getPlayer, setPlayer};