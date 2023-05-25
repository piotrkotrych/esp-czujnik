//create fetch hook with interval
//this hook will be used in the components to fetch data from the server
//with an interval to update the data

import { useState, useEffect } from "react";

export default function useFetchInterval(url, interval) {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    const abortCont = new AbortController();
    async function fetchData() {
      try {
        const response = await fetch(url);
        const json = await response.json();
        setData(json);
        setLoading(false);
      } catch (error) {
        setError(error);
      }
    }

    fetchData();
    const intervalId = setInterval(() => fetchData(), interval);
    return () => {
      abortCont.abort();
      clearInterval(intervalId);
    };
  }, [url, interval]);

  return { data, loading, error };
}
